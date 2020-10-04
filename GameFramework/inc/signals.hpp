#pragma once

//          Copyright Jeremiah van Oosten 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//

 /**
  *  @file signals.hpp
  *  @date March 24, 2020
  *  @author Jeremiah van Oosten
  *
  *  @brief The signals header file.
  *  @see https://www.boost.org/doc/libs/1_72_0/doc/html/signals2.html
  *  @see https://github.com/palacaze/sigslot
  */

#include "optional.hpp" // for opt::optional
#include <atomic>       // for std::atomic_bool
#include <cstddef>      // for std::size_t and std::nullptr_t
#include <exception>    // for std::exception
#include <functional>   // for std::reference_wrapper
#include <memory>       // for std::unique_ptr
#include <mutex>        // for std::mutex, and std::lock_guard
#include <tuple>        // for std::tuple, and std::make_tuple
#include <type_traits>  // for std::decay, and std::enable_if
#include <utility>      // for std::declval.
#include <vector>       // for std::vector

namespace sig
{
    // An exception of type not_comparable_exception is thrown
    // if one tries to compare non comparable function types.
    class not_comparable_exception : public std::exception
    {};

    // Pointers that can be converted to a weak pointer concept for 
    // tracking purposes must implement the to_weak() function in order
    // to make use of Argument-dependent lookup (ADL) and to convert
    // it to a type whose lifetime can be tracked by the slot.
    template<typename T>
    std::weak_ptr<T> to_weak(std::weak_ptr<T> w)
    {
        return w;
    }

    template<typename T>
    std::weak_ptr<T> to_weak(std::shared_ptr<T> s)
    {
        return s;
    }

    // Forward declare signal class so that it can be 
    // a friend of a class in a nested namespace.
    template<typename, typename>
    class signal;

    namespace detail
    {
        namespace traits
        {
            // Since C++17
            // @see https://en.cppreference.com/w/cpp/types/void_t
            template<typename... Ts>
            struct make_void
            {
                typedef void type;
            };

            template<typename... Ts>
            using void_t = typename make_void<Ts...>::type;

            // Since C++14
            template<typename T>
            using decay_t = typename std::decay<T>::type;

            // Since C++14
            template< bool B, typename T = void >
            using enable_if_t = typename std::enable_if<B, T>::type;

            // Since C++14
            template<bool B, typename T, typename F>
            using conditional_t = typename std::conditional<B, T, F>::type;

            // Since C++14
            template<typename T>
            using remove_cv_t = typename std::remove_cv<T>::type;

            // Since C++14
            template<typename T>
            using remove_reference_t = typename std::remove_reference<T>::type;

            // Since C++20
            template<typename T>
            struct remove_cvref {
                using type = remove_cv_t<remove_reference_t<T>>;
            };

            template<typename T>
            using remove_cvref_t = typename remove_cvref<T>::type;

            // Type used to indicate SFINAE success.
            template<typename T>
            struct success_type
            {
                using type = T;
            };

            // Type used to indicate SFINAE failure.
            struct failure_type
            {};

            // Detect reference wrapper
            // @see https://stackoverflow.com/questions/40430692/how-to-detect-stdreference-wrapper-in-c-at-compile-time
            template <typename T>
            struct is_reference_wrapper : std::false_type {};
            template <typename U>
            struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

            // Test for equality comparable
            // @see C++ Templates: The Complete Guide (David Vandevoorde, et. al., 2018)
            template<typename T>
            struct is_equality_comparable
            {
            private:
                // Test convertibility of == and !(==) to bool:
                static void* conv(bool);

                template<typename U>
                static std::true_type test(
                    decltype(conv(std::declval<const U&>() == std::declval<const U&>())),
                    decltype(conv(!(std::declval<const U&>() == std::declval<const U&>())))
                );

                // Fallback
                template<typename U>
                static std::false_type test(...);

            public:
                static constexpr bool value = decltype(test<T>(nullptr, nullptr))::value;
            };

            // Detect if type supports weak_ptr semantics.
            template<typename T, typename = void>
            struct is_weak_ptr : std::false_type
            {};

            template<typename T>
            struct is_weak_ptr<T, void_t<decltype(std::declval<T>().expired()),
                decltype(std::declval<T>().lock()),
                decltype(std::declval<T>().reset())>>
                : std::true_type
            {};

            // Test to see if a type can be converted to a weak_ptr type.
            template <typename T, typename = void>
            struct is_weak_ptr_convertable : std::false_type
            {};

            template <typename T>
            struct is_weak_ptr_convertable<T, void_t<decltype(to_weak(std::declval<T>()))>>
                : is_weak_ptr<decltype(to_weak(std::declval<T>()))>
            {};

            // Primary template for extracting function traits.
            template<typename Func>
            struct function_traits;

            template<typename R, typename... Args>
            struct function_traits<R(Args...)>
            {
                using result_type = R;
            };

            // Used by result_of, invoke etc. to unwrap a reference_wrapper.
            template<typename T, typename U = remove_cvref_t<T>>
            struct inv_unwrap
            {
                using type = T;
            };

            template<typename T, typename U>
            struct inv_unwrap<T, std::reference_wrapper<U>>
            {
                using type = U&;
            };

            template<typename T>
            using inv_unwrap_t = typename inv_unwrap<T>::type;

            // Primary template for result_of.
            // 
            template<typename Func>
            struct result_of;

            // Invoke tags.
            struct invoke_func{};
            struct invoke_memfun_ref{};
            struct invoke_memfun_deref{};
            struct invoke_memobj_ref{};
            struct invoke_memobj_deref{};

            // Associate a tag with a specialization of success_type
            template<typename T, typename Tag>
            struct result_of_success : success_type<T>
            {
                using invoke_type = Tag;
            };

            // Determine result of calling a function object.
            struct result_of_func_impl
            {
                template<typename Func, typename... Args>
                static result_of_success<decltype(std::declval<Func>()(std::declval<Args>()...)), invoke_func>
                test(int);

                // Fallback on failure.
                template<typename...>
                static failure_type test(...);
            };

            // Determine the result of calling a pointer to member function
            // for reference types.
            struct result_of_memfun_ref_impl
            {
                template<typename Func, typename T, typename... Args>
                static result_of_success<decltype((std::declval<T>().*std::declval<Func>())(std::declval<Args>()...)), invoke_memfun_ref>
                test(int);

                // Fallback on failure.
                template<typename...>
                static failure_type test(...);
            };

            template<typename Func, typename Arg, typename... Args>
            struct result_of_memfun_ref : private result_of_memfun_ref_impl
            {
                using type = decltype(test<Func, Arg, Args...>(0));
            };

            template<typename Func, typename Arg, typename... Args>
            using result_of_memfun_ref_t = typename result_of_memfun_ref<Func, Arg, Args...>::type;

            // Determine the result of calling a pointer to member function
            // for pointer types.
            struct result_of_memfun_deref_impl
            {
                template<typename Func, typename T, typename... Args>
                static result_of_success<decltype(((*std::declval<T>()).*std::declval<Func>())(std::declval<Args>()...)), invoke_memfun_deref>
                test(int);

                // Fallback on failure.
                template<typename...>
                static failure_type test(...);
            };

            template<typename Func, typename Arg, typename... Args>
            struct result_of_memfun_deref : private result_of_memfun_deref_impl
            {
                using type = decltype(test<Func, Arg, Args...>(0));
            };

            template<typename Func, typename Arg, typename... Args>
            using result_of_memfun_deref_t = typename result_of_memfun_deref<Func, Arg, Args...>::type;

            // Determine the result of a pointer to member data.
            // for reference types.
            struct result_of_memobj_ref_impl
            {
                template<typename F, typename T>
                static result_of_success<decltype(std::declval<T>().*std::declval<F>()), invoke_memobj_ref>
                test(int);

                template<typename, typename>
                static failure_type test(...);
            };

            template<typename MemPtr, typename Arg>
            struct result_of_memobj_ref : private result_of_memobj_ref_impl
            {
                using type = decltype(test<MemPtr, Arg>(0));
            };

            template<typename MemPtr, typename Arg>
            using result_of_memobj_ref_t = typename result_of_memobj_ref<MemPtr, Arg>::type;

            // Determine the result of a pointer to member data for
            // pointer types.
            struct result_of_memobj_deref_impl
            {
                template<typename F, typename T>
                static result_of_success<decltype((*std::declval<T>()).*std::declval<F>()), invoke_memobj_deref>
                test(int);

                template<typename, typename>
                static failure_type test(...);
            };

            template<typename MemPtr, typename Arg>
            struct result_of_memobj_deref : private result_of_memobj_deref_impl
            {
                using type = decltype(test<MemPtr, Arg>(0));
            };

            template<typename MemPtr, typename Arg>
            using result_of_memobj_deref_t = typename result_of_memobj_deref<MemPtr, Arg>::type;

            template<typename MemPtr, typename Arg>
            struct result_of_memobj;

            template<typename Type, typename Class, typename Arg>
            struct result_of_memobj<Type Class::*, Arg>
            {
                using ArgVal = remove_cvref_t<Arg>;
                using MemPtr = Type Class::*;
                using type = conditional_t<std::is_same<ArgVal, Class>::value || std::is_base_of<Class, ArgVal>::value,
                    result_of_memobj_ref_t<MemPtr, Arg>,
                    result_of_memobj_deref_t<MemPtr, Arg>>;
            };

            template<typename MemPtr, typename Arg>
            using result_of_memobj_t = typename result_of_memobj<MemPtr, Arg>::type;

            template<typename MemPtr, typename Class, typename... Args>
            struct result_of_memfun;

            template<typename Func, typename Class, typename Arg, typename... Args>
            struct result_of_memfun<Func Class::*, Arg, Args...>
            {
                using ArgVal = remove_reference_t<Arg>;
                using MemPtr = Func Class::*;
                using type = conditional_t<std::is_base_of<Class, ArgVal>::value,
                    result_of_memfun_ref_t<MemPtr, Arg, Args...>,
                    result_of_memfun_deref_t<MemPtr, Arg, Args...>>;
            };

            template<typename Func, typename Class, typename Arg, typename... Args>
            using result_of_memfun_t = typename result_of_memfun<Func, Class, Arg, Args...>::type;

            template<bool IsMemberObjectPointer, bool IsMemberFunctionPointer, 
                typename Func, typename... Args>
            struct result_of_impl
            {
                using type = failure_type;
            };

            template<typename MemPtr, typename Arg>
            struct result_of_impl<true, false, MemPtr, Arg> : result_of_memobj<decay_t<MemPtr>, inv_unwrap_t<Arg>>
            {};

            template<typename MemPtr, typename Arg, typename... Args>
            struct result_of_impl<false, true, MemPtr, Arg, Args...> : result_of_memfun<decay_t<MemPtr>, inv_unwrap_t<Arg>, Args...>
            {};

            template<typename Func, typename... Args>
            struct result_of_impl<false, false, Func, Args...> : private result_of_func_impl
            {
                using type = decltype(test<Func, Args...>(0));
            };

            template<typename Func, typename... Args>
            struct invoke_result : public result_of_impl<
                std::is_member_object_pointer<remove_reference_t<Func>>::value,
                std::is_member_function_pointer<remove_reference_t<Func>>::value,
                Func, Args...>::type
            {};

            template<typename Func, typename... Args>
            using invoke_result_t = typename invoke_result<Func, Args...>::type;

            // Detect if a function type is invocable given a set of arguments.

            // Primary template for invalid INVOKE expressions.
            template<typename Result, typename R, bool = std::is_void<R>::value, typename = void>
            struct is_invocable_impl : std::false_type
            {};

            // Valid INVOKE and INVOKE<void> expressions
            template<typename Result, typename R>
            struct is_invocable_impl<Result, R, true, void_t<typename Result::type>> : std::true_type
            {};

            // Valid INVOKE<R> expressions.
            template<typename Result, typename R>
            struct is_invocable_impl<Result, R, false, void_t<typename Result::type>>
            {
            private:
                // The type of the INVOKE expression.
                static typename Result::type get();

                template<typename T>
                static void conv(T);

                // This overload is viable if INVOKE(f, args...) can convert to T
                template<typename T, typename = decltype(conv<T>(get()))>
                static std::true_type test(int);

                // Fallback if failure
                template<typename T>
                static std::false_type test(...);

            public:
                using type = decltype(test<R>(0));
            };

            template<typename Func, typename... Args>
            struct is_invocable : is_invocable_impl<invoke_result<Func, Args...>, void>::type
            {};

            template<typename R, typename Func, typename... Args>
            struct is_invocable_r : is_invocable_impl<invoke_result<Func, Args...>, R>::type
            {};

        } // namespace traits

        // Use is_equality_compareable to try to perform the equality check
        // (if it is valid for the given type).
        template<typename T, bool = traits::is_equality_comparable<T>::value>
        struct try_equals
        {
            static bool equals(const T& t1, const T& t2)
            {
                return t1 == t2;
            }
        };

        // Partial specialization if type is not equality comparable.
        // In this case, throw an exception to indicate that the type is not 
        // equality comparable.
        template<typename T>
        struct try_equals<T, false>
        {
            static bool equals(const T&, const T&)
            {
                throw sig::not_comparable_exception();
            }
        };

        // Primary template
        // Invokes a function object.
        // @see https://en.cppreference.com/w/cpp/types/result_of
        template<typename>
        struct invoke_helper
        {
            // Call a function object.
            template<typename Func, typename... Args>
            static auto call(Func&& f, Args&&... args) -> decltype(std::forward<Func>(f)(std::forward<Args>(args)...))
            {
                return std::forward<Func>(f)(std::forward<Args>(args)...);
            }
        };

        // Invoke a pointer to member function or pointer to member data.
        // @see https://en.cppreference.com/w/cpp/types/result_of
        template<typename Type, typename Base>
        struct invoke_helper<Type Base::*>
        {
            // Get a reference type.
            template<typename T, typename Td = traits::decay_t<T>,
                typename = traits::enable_if_t<std::is_base_of<Base, Td>::value>>
                static auto get(T&& t) -> T&&
            {
                return t;
            }

            // Get a std::reference_wrapper
            template<typename T, typename Td = traits::decay_t<T>,
                typename = traits::enable_if_t<traits::is_reference_wrapper<Td>::value>>
                static auto get(T&& t) -> decltype(t.get())
            {
                return t.get();
            }

            // Get a pointer or pointer-like object (like smart_ptr, or unique_ptr)
            template<typename T, typename Td = traits::decay_t<T>,
                typename = traits::enable_if_t<!std::is_base_of<Base, Td>::value>,
                typename = traits::enable_if_t<!traits::is_reference_wrapper<Td>::value>>
                static auto get(T&& t) -> decltype(*std::forward<T>(t))
            {
                return *std::forward<T>(t);
            }

            // Call a pointer to a member function.
            template<typename T, typename... Args, typename Type1,
                typename = traits::enable_if_t<std::is_function<Type1>::value>>
                static auto call(Type1 Base::* pmf, T&& t, Args&&... args)
                -> decltype((get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...))
            {
                return (get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...);
            }

            // Call a pointer to member data.
            template<typename T>
            static auto call(Type Base::* pmd, T&& t)
                -> decltype(get(std::forward<T>(t)).*pmd)
            {
                return get(std::forward<T>(t)).*pmd;
            }
        };

        /**
         * A copy-on-write template class to avoid unnecessary copies of
         * data unless the data will be modified. This greatly improves
         * the performance of the signal class in a multi-threaded environment
         * since read-only copies only require a shared pointer copy.
         * The copy-on-write pointer has similar semantics to shared pointers.
         *
         * @see API Design for C++, Martin Reddy (Elsevier, 2011).
         */
        template<typename T>
        class cow_ptr
        {
        public:
            using value_type = T;
            using pointer_type = std::shared_ptr<T>;

            constexpr cow_ptr() noexcept = default;
            ~cow_ptr() = default;

            constexpr explicit cow_ptr(T* other) noexcept
                : m_Ptr(other)
            {}

            constexpr cow_ptr(const cow_ptr& other) noexcept
                : m_Ptr(other.m_Ptr)
            {}

            constexpr cow_ptr(cow_ptr&& other) noexcept
                : m_Ptr(std::move(other.m_Ptr))
            {}

            // Copy assignment operator.
            cow_ptr& operator=(const cow_ptr& other) noexcept
            {
                if (this != &other)
                {
                    *this = cow_ptr(other);
                }

                return *this;
            }

            // Move assignment operator.
            cow_ptr& operator=(cow_ptr&& other) noexcept
            {
                m_Ptr = std::move(other.m_Ptr);
                return *this;
            }

            // Assign from other value.
            cow_ptr& operator=(T* other) noexcept
            {
                m_Ptr = pointer_type(other);
                return *this;
            }

            // Non-const dereference operator.
            // Will create a copy of the underlying object.
            T& operator*()
            {
                detach();
                return *m_Ptr;
            }

            // Const dereference operator.
            // No copy is made of the underlying object
            const T& operator*() const noexcept
            {
                return *m_Ptr;
            }

            // Non-const pointer dereference operator.
            // Will create a copy of the underlying object.
            T* operator->()
            {
                detach();
                return m_Ptr.get();
            }

            // Const pointer dereference operator.
            // No copy is made of the underlying object.
            const T* operator->() const
            {
                return m_Ptr.get();
            }

            // Non-const implicit conversion of underlying type.
            // A copy will be created of the underlying object.
            operator T* ()
            {
                detach();
                return m_Ptr.get();
            }

            // Const implicit conversion of underlying type.
            // No copy is made of the underlying object.
            operator const T* () const
            {
                return m_Ptr.get();
            }

            T* data()
            {
                detach();
                return m_Ptr.get();
            }

            const T* data() const
            {
                return m_Ptr.get();
            }

            // Get read-only reference to internal value.
            const T& read() const
            {
                return *m_Ptr;
            }

            // Get writable reference to internal value.
            T& write()
            {
                detach();
                return *m_Ptr;
            }

            template<typename U>
            bool operator==(const cow_ptr<U>& rhs) const noexcept
            {
                return m_Ptr == rhs.m_Ptr;
            }

            template<typename U>
            bool operator!=(const cow_ptr<U>& rhs) const noexcept
            {
                return m_Ptr != rhs.m_Ptr;
            }

            // Explicit bool conversion to check for a valid
            // internal value.
            explicit operator bool() const noexcept
            {
                return bool(m_Ptr);
            }

        private:
            void detach()
            {
                if (m_Ptr && m_Ptr.use_count() > 1)
                {
                    // Detach from the shared pointer
                    // creating a new instance of the stored object.
                    *this = cow_ptr(new T(*m_Ptr));
                }
            }

            pointer_type m_Ptr;
        };

        // Utility function to create a cow_ptr.
        template<typename T, typename... Args>
        cow_ptr<T> make_cow(Args&&... args)
        {
            return cow_ptr<T>(new T(std::forward<Args>(args)...));
        }

        template<typename T, typename U, typename... Args>
        cow_ptr<T> make_cow(std::initializer_list<U> il, Args&&... args)
        {
            return cow_ptr<T>(new T(il, std::forward<Args>(args)...));
        }

        /**
         * Slot state is used as both a non-template base class for slot_impl
         * as well as storing connection information about the slot.
         */
        class slot_state
        {
        public:
            constexpr slot_state() noexcept
                : m_Index(0)
                , m_Connected(true)
                , m_Blocked(false)
            {}

            // Atomic variables are not CopyConstructible.
            // @see https://en.cppreference.com/w/cpp/atomic/atomic/atomic
            slot_state(const slot_state& s) noexcept
                : m_Index(s.m_Index)
                , m_Connected(s.m_Connected.load())
                , m_Blocked(s.m_Blocked.load())
            {}

            slot_state(slot_state&& s) noexcept
                : m_Index(s.m_Index)
                , m_Connected(s.m_Connected.load())
                , m_Blocked(s.m_Blocked.load())
            {}

            virtual ~slot_state() = default;

            slot_state& operator=(const slot_state& s) noexcept
            {
                m_Index = s.m_Index;
                m_Connected = s.m_Connected.load();
                m_Blocked = s.m_Blocked.load();

                return *this;
            }

            slot_state& operator=(slot_state&& s) noexcept
            {
                m_Index = s.m_Index;
                m_Connected.store(s.m_Connected.load());
                m_Blocked.store(s.m_Blocked.load());

                return *this;
            }

            virtual bool connected() const noexcept
            {
                return m_Connected;
            }

            bool disconnect() noexcept
            {
                return m_Connected.exchange(false);
            }

            bool blocked() const noexcept
            {
                return m_Blocked;
            }

            void block() noexcept
            {
                m_Blocked = true;
            }

            void unblock() noexcept
            {
                m_Blocked = false;
            }

            std::size_t& index()
            {
                return m_Index;
            }

        private:
            std::size_t m_Index;
            std::atomic_bool m_Connected;
            std::atomic_bool m_Blocked;
        };

        // Base class for slot implementations.
        template<typename R, typename... Args>
        class slot_impl : public slot_state
        {
        public:
            virtual ~slot_impl() = default;
            virtual slot_impl* clone() const = 0;
            virtual bool equals(const slot_impl* s) const = 0;
            virtual opt::optional<R> operator()(Args&&... args) = 0;
        };

        // Slot implementation for callable function objects (Functors)
        template<typename R, typename Func, typename... Args>
        class slot_func : public slot_impl<R, Args...>
        {
        public:
            using fuction_type = traits::decay_t<Func>;

            slot_func(const slot_func&) = default;  // Copy constructor.

            slot_func(Func&& func)
                : m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<R, Args...>* clone() const override
            {
                return new slot_func(*this);
            }

            virtual bool equals(const slot_impl<R, Args...>* s) const override
            {
                if (auto sfunc = dynamic_cast<const slot_func*>(s))
                {
                    return try_equals<fuction_type>::equals(m_Func, sfunc->m_Func);
                }

                return false;
            }

            virtual opt::optional<R> operator()(Args&&... args) override
            {
                return invoke_helper<fuction_type>::call(m_Func, std::forward<Args>(args)...);
            }

        private:
            fuction_type m_Func;
        };

        // Specialization for void return types.
        template<typename Func, typename... Args>
        class slot_func<void, Func, Args...> : public slot_impl<void, Args...>
        {
        public:
            using fuction_type = traits::decay_t<Func>;

            slot_func(const slot_func&) = default;  // Copy constructor.

            slot_func(Func&& func)
                : m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<void, Args...>* clone() const override
            {
                return new slot_func(*this);
            }

            virtual bool equals(const slot_impl<void, Args...>* s) const override
            {
                if (auto sfunc = dynamic_cast<const slot_func*>(s))
                {
                    return try_equals<fuction_type>::equals(m_Func, sfunc->m_Func);
                }

                return false;
            }

            virtual opt::optional<void> operator()(Args&&... args) override
            {
                return (invoke_helper<fuction_type>::call(m_Func, std::forward<Args>(args)...), opt::nullopt);
            }

        private:
            fuction_type m_Func;
        };

        // Slot implementation for pointer to member function and
        // pointer to member data.
        template<typename R, typename Func, typename Ptr, typename... Args>
        class slot_pmf : public slot_impl<R, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<Ptr>;

            slot_pmf(const slot_pmf&) = default;

            slot_pmf(Func&& func, Ptr&& ptr)
                : m_Ptr{ std::forward<Ptr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<R, Args...>* clone() const override
            {
                return new slot_pmf(*this);
            }

            virtual bool equals(const slot_impl<R, Args...>* s) const override
            {
                if (auto spmf = dynamic_cast<const slot_pmf*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmf->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmf->m_Func);
                }

                return false;
            }

            virtual opt::optional<R> operator()(Args&&... args) override
            {
                return invoke_helper<function_type>::call(m_Func, m_Ptr, std::forward<Args>(args)...);
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // Slot implementation for pointer to member function and
        // pointer to member data.
        // Specialized on void return value.
        template<typename Func, typename Ptr, typename... Args>
        class slot_pmf<void, Func, Ptr, Args...> : public slot_impl<void, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<Ptr>;

            slot_pmf(const slot_pmf&) = default;

            slot_pmf(Func&& func, Ptr&& ptr)
                : m_Ptr{ std::forward<Ptr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual slot_impl<void, Args...>* clone() const override
            {
                return new slot_pmf(*this);
            }

            virtual bool equals(const slot_impl<void, Args...>* s) const override
            {
                if (auto spmf = dynamic_cast<const slot_pmf*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmf->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmf->m_Func);
                }

                return false;
            }

            virtual opt::optional<void> operator()(Args&&... args) override
            {
                invoke_helper<function_type>::call(m_Func, m_Ptr, std::forward<Args>(args)...);
                return {};
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // Slot implementation for pointer to member function that automatically 
        // tracks the lifetime of a supplied object through a weak pointer in 
        // order to disconnect the slot on object destruction.
        template<typename R, typename Func, typename WeakPtr, typename... Args>
        class slot_pmf_tracked : public slot_impl<R, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<WeakPtr>;

            slot_pmf_tracked(const slot_pmf_tracked&) = default;

            slot_pmf_tracked(Func&& func, WeakPtr&& ptr)
                : m_Ptr{ std::forward<WeakPtr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual bool connected() const noexcept override
            {
                return !m_Ptr.expired() && slot_state::connected();
            }

            virtual slot_impl<R, Args...>* clone() const override
            {
                return new slot_pmf_tracked(*this);
            }

            virtual bool equals(const slot_impl<R, Args...>* s) const override
            {
                if (auto spmft = dynamic_cast<const slot_pmf_tracked*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmft->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmft->m_Func);
                }

                return false;
            }

            virtual opt::optional<R> operator()(Args&&... args) override
            {
                auto sp = m_Ptr.lock();
                if (!sp)
                {
                    this->disconnect();
                    return {};
                }

                if (this->connected())
                {
                    return invoke_helper<function_type>::call(m_Func, sp, std::forward<Args>(args)...);
                }

                return {};
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // Slot implementation for pointer to member function that automatically 
        // tracks the lifetime of a supplied object through a weak pointer in 
        // order to disconnect the slot on object destruction.
        // Partial specialization for void return types.
        template<typename Func, typename WeakPtr, typename... Args>
        class slot_pmf_tracked<void, Func, WeakPtr, Args...> : public slot_impl<void, Args...>
        {
        public:
            using function_type = traits::decay_t<Func>;
            using pointer_type = traits::decay_t<WeakPtr>;

            slot_pmf_tracked(const slot_pmf_tracked&) = default;

            slot_pmf_tracked(Func&& func, WeakPtr&& ptr)
                : m_Ptr{ std::forward<WeakPtr>(ptr) }
                , m_Func{ std::forward<Func>(func) }
            {}

            virtual bool connected() const noexcept override
            {
                return !m_Ptr.expired() && slot_state::connected();
            }

            virtual slot_impl<void, Args...>* clone() const override
            {
                return new slot_pmf_tracked(*this);
            }

            virtual bool equals(const slot_impl<void, Args...>* s) const override
            {
                if (auto spmft = dynamic_cast<const slot_pmf_tracked*>(s))
                {
                    return try_equals<pointer_type>::equals(m_Ptr, spmft->m_Ptr) &&
                        try_equals<function_type>::equals(m_Func, spmft->m_Func);
                }

                return false;
            }

            virtual opt::optional<void> operator()(Args&&... args) override
            {
                auto sp = m_Ptr.lock();
                if (!sp)
                {
                    this->disconnect();
                }

                if (this->connected())
                {
                    invoke_helper<function_type>::call(m_Func, sp, std::forward<Args>(args)...);
                }

                return {};
            }

        private:
            pointer_type m_Ptr;
            function_type m_Func;
        };

        // since C++14
        // @see https://en.cppreference.com/w/cpp/utility/integer_sequence
        // @see https://gist.github.com/ntessore/dc17769676fb3c6daa1f
        // Integer sequence is used to unpack a tuple. This is required for the 
        // slot iterator.
        template<typename T, T... Ints>
        struct integer_sequence
        {
            using value_type = T;
            static constexpr std::size_t size()
            {
                return sizeof...(Ints);
            }
        };

        template<std::size_t... Ints>
        using index_sequence = integer_sequence<std::size_t, Ints...>;

        template<typename T, std::size_t N, T... Is>
        struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Is...>
        {};

        // Specialization for integer sequences of 0 length.
        template<typename T, T... Is>
        struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...>
        {};

        template<std::size_t N>
        using make_index_sequence = make_integer_sequence<std::size_t, N>;

        template<typename... T>
        using index_sequence_for = make_index_sequence<sizeof...(T)>;

        // The slot_iterator is a wrapper for the actual container that 
        // contains a list of slots to be invoked. When the slot_iterator
        // is dereferenced, it must invoke the slot that is referenced by the 
        // current internal iterator and return the result of invoking the function.
        template<typename T, typename InputIterator, typename... Args>
        class slot_iterator
        {
        public:
            // These are required for iterators:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            using args_type = std::tuple<Args...>;
            using args_sequence = make_index_sequence<sizeof...(Args)>;

            slot_iterator(InputIterator iter, args_type& args)
                : m_Iter(iter)
                , m_Args(args)
            {}

            slot_iterator(const slot_iterator&) = default;
            slot_iterator(slot_iterator&&) = default;

            // Pre-increment operator.
            slot_iterator& operator++()
            {
                ++m_Iter;
                return *this;
            }

            // Post-increment operator
            slot_iterator operator++(int)
            {
                slot_iterator tmp(*this);
                ++m_Iter;
                return tmp;
            }

            bool operator==(const slot_iterator& other) const
            {
                return m_Iter == other.m_Iter;
            }

            bool operator!=(const slot_iterator& other) const
            {
                return m_Iter != other.m_Iter;
            }

            // Invoke the slot referenced by the internal iterator.
            opt::optional<T> operator*()
            {
                return do_invoke(args_sequence());
            }

        private:
            template<std::size_t... Is>
            constexpr opt::optional<T> do_invoke(index_sequence<Is...>)
            {
                // Unpack tuple arguments and invoke slot.
                return (**m_Iter)(std::forward<Args>(std::get<Is>(m_Args))...);
            }

            InputIterator m_Iter;
            args_type& m_Args;
        };

        // Specialization for void return types.
        template<typename InputIterator, typename... Args>
        class slot_iterator<void, InputIterator, Args...>
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = void;
            using difference_type = std::ptrdiff_t;
            using pointer = void;
            using reference = void;

            using args_type = std::tuple<Args...>;
            using args_sequence = make_index_sequence<sizeof...(Args)>;

            slot_iterator(InputIterator iter, args_type& args)
                : m_Iter(iter)
                , m_Args(args)
            {}

            slot_iterator(const slot_iterator&) = default;
            slot_iterator(slot_iterator&&) = default;

            // Pre-increment operator.
            slot_iterator& operator++()
            {
                ++m_Iter;
                return *this;
            }

            // Post-increment operator
            slot_iterator operator++(int)
            {
                slot_iterator tmp(*this);
                ++m_Iter;
                return tmp;
            }

            bool operator==(const slot_iterator& other) const
            {
                return m_Iter == other.m_Iter;
            }

            bool operator!=(const slot_iterator& other) const
            {
                return m_Iter != other.m_Iter;
            }

            opt::optional<void> operator*()
            {
                return do_invoke(args_sequence());
            }

        private:
            template<std::size_t... Is>
            constexpr opt::optional<void> do_invoke(index_sequence<Is...>)
            {
                // Unpack tuple arguments and invoke slot.
                (**m_Iter)(std::forward<Args>(std::get<Is>(m_Args))...);
                return {};
            }

            InputIterator m_Iter;
            args_type& m_Args;
        };

        // Base type for slots.
        // This is required for signal_base to be able to pass
        // a slot to a signal through the signal_base interface.
        class slot_base
        {
            // The signal class needs access to the index method.
            template<typename, typename>
            friend class sig::signal;
            virtual std::size_t& index() = 0;

        public:
            virtual void block() = 0;
            virtual void unblock() = 0;
            virtual bool connected() const noexcept = 0;
            virtual bool blocked() const noexcept = 0;
            virtual bool disconnect() = 0;
        };

        // Base type for the signal class.
        // Allows slots to disconnect from a signal.
        class signal_base
        {
        public:
            virtual ~signal_base() = default;
            virtual void remove_slot(slot_base& slot) = 0;
        };

    } // namespace detail

    // Primary slot template
    template<typename Func>
    class slot;

    // TODO: Refactor slot to derive from detail::slot_state
    // Specialization for function objects.
    template<typename R, typename... Args>
    class slot<R(Args...)> : public detail::slot_base
    {
        using impl = detail::slot_impl<R, Args...>;

    public:
        // Default constructor.
        constexpr slot() noexcept
            : m_pImpl{ nullptr }
            , m_pSignal{ nullptr }
        {}

        // Slot that takes a function object.
        template<typename Func>
        slot(Func&& func, detail::signal_base* signal = nullptr)
            : m_pImpl{ new detail::slot_func<R, Func, Args...>(std::forward<Func>(func)) }
            , m_pSignal{ signal }
        {}

        // Slot that takes a pointer to member function or pointer to member data.
        template<typename Func, typename Ptr>
        slot(Func&& func, Ptr&& ptr, detail::signal_base* signal = nullptr,
            detail::traits::enable_if_t<!detail::traits::is_weak_ptr_convertable<Ptr>::value, void*> = nullptr)
            : m_pImpl{ new detail::slot_pmf<R, Func, Ptr, Args...>(std::forward<Func>(func), std::forward<Ptr>(ptr)) }
            , m_pSignal{ signal }
        {}

        template<typename Func, typename Ptr>
        slot(Func&& func, Ptr&& ptr, detail::signal_base* signal = nullptr,
            detail::traits::enable_if_t<detail::traits::is_weak_ptr_convertable<Ptr>::value, void*> = nullptr)
            : m_pImpl{ new detail::slot_pmf_tracked<R, Func, decltype(to_weak(std::forward<Ptr>(ptr))), Args...>(std::forward<Func>(func), to_weak(std::forward<Ptr>(ptr))) }
            , m_pSignal{ signal }
        {}

        // Copy constructor.
        slot(const slot& copy, detail::signal_base* signal = nullptr)
            : m_pImpl{ copy.m_pImpl->clone() }
            , m_pSignal{ signal ? signal : copy.m_pSignal }
        {}

        // Explicit parameterized constructor.
        explicit slot(std::unique_ptr<impl> pImpl)
            : m_pImpl{ pImpl }
            , m_pSignal{ nullptr }
        {}

        // Move constructor.
        slot(slot&& other)
            : m_pImpl{ std::move(other.m_pImpl) }
            , m_pSignal{ other.m_pSignal }
        {
            other.m_pSignal = nullptr;
        }

        // Assignment operator.
        slot& operator=(const slot& other)
        {
            if (&other != this)
            {
                m_pImpl.swap(other.m_pImpl->clone());
                m_pSignal = other.m_pSignal;
            }
            return *this;
        }

        // Move assignment operator.
        slot& operator=(slot&& other) noexcept
        {
            m_pImpl = std::move(other.m_pImpl);
            m_pSignal = other.m_pSignal;
            other.m_pSignal = nullptr;

            return *this;
        }

        // Explicit conversion to bool.
        explicit operator bool() const
        {
            return m_pImpl != nullptr;
        }

        // Equality operator
        friend bool operator==(const slot& s1, const slot& s2)
        {
            if (!s1 || !s2)
            {
                return !s1 && !s2;
            }

            return s1.m_pImpl->equals(s2.m_pImpl.get());
        }

        // Inequality operator
        friend bool operator!=(const slot& s1, const slot& s2)
        {
            return !(s1 == s2);
        }

        bool connected() const noexcept
        {
            return m_pImpl && m_pImpl->connected();
        }

        bool disconnect() noexcept
        {
            if (m_pImpl && m_pSignal && m_pImpl->disconnect() )
            {
                m_pSignal->remove_slot(*this);
                return true;
            }

            return false;
        }

        bool blocked() const noexcept
        {
            return m_pImpl && m_pImpl->blocked();
        }

        void block() noexcept
        {
            if (m_pImpl)
                m_pImpl->block();
        }

        void unblock() noexcept
        {
            if (m_pImpl)
                m_pImpl->unblock();
        }

        // Invoke the slot.
        opt::optional<R> operator()(Args&&... args)
        {
            if (!blocked() && connected())
            {
                return (*m_pImpl)(std::forward<Args>(args)...);
            }

            return {};
        }

    private:
        // Signals need to access the state of the slots.
        template<typename, typename>
        friend class signal;

        virtual std::size_t& index() override
        {
            return m_pImpl->index();
        }

        // Query the state of the slot.
        constexpr detail::slot_state& state()
        {
            return *m_pImpl;
        }

        std::unique_ptr<impl> m_pImpl;              // Pointer to implementation
        detail::signal_base* m_pSignal;             // The signal this slot belongs to.
    };

    // Shared pointer to a slot.
    using slot_ptr = std::shared_ptr<detail::slot_base>;

    // Weak pointer to a slot.
    using slot_wptr = std::weak_ptr<detail::slot_base>;

    /**
     * An RAII object that blocks connections until destruction.
     */
    class connection_blocker
    {
    public:
        using slot_type = slot_wptr;

        connection_blocker() noexcept = default;
        ~connection_blocker() noexcept
        {
            release();
        }

        connection_blocker(const connection_blocker&) noexcept = delete;
        connection_blocker(connection_blocker&& c) noexcept
            : m_Slot{ std::move(c.m_Slot) }
        {}

        connection_blocker& operator=(const connection_blocker&) = delete;
        connection_blocker& operator=(connection_blocker&& c) noexcept
        {
            release();
            m_Slot = std::move(c.m_Slot);
            return *this;
        }

        void swap(connection_blocker& other) noexcept
        {
            std::swap(m_Slot, other.m_Slot);
        }

    private:
        friend class connection;

        explicit connection_blocker(slot_type slot) noexcept
            : m_Slot{ std::move(slot) }
        {
            if (auto s = m_Slot.lock())
            {
                s->block();
            }
        }

        void release() noexcept
        {
            if (auto s = m_Slot.lock())
            {
                s->unblock();
            }
        }

        slot_type m_Slot;
    };

    inline void swap(connection_blocker& cb1, connection_blocker& cb2) noexcept
    {
        cb1.swap(cb2);
    }

    /**
     * Connection object allows for managing slot connections.
     */
    class connection
    {
    public:
        using slot_type = slot_wptr;

        connection() noexcept = default;
        virtual ~connection() = default;

        connection(const connection&) noexcept = default;
        connection(connection&&) noexcept = default;
        connection& operator=(const connection&) noexcept = default;
        connection& operator=(connection&&) noexcept = default;

        bool valid() const noexcept
        {
            return !m_Slot.expired();
        }

        bool connected() const noexcept
        {
            const auto s = m_Slot.lock();
            return s && s->connected();
        }

        bool disconnect() noexcept
        {
            auto s = m_Slot.lock();
            return s && s->disconnect();
        }

        bool blocked() const noexcept
        {
            const auto s = m_Slot.lock();
            return s && s->blocked();
        }

        void block() noexcept
        {
            if (auto s = m_Slot.lock())
            {
                s->block();
            }
        }

        void unblock() noexcept
        {
            if (auto s = m_Slot.lock())
            {
                s->unblock();
            }
        }

        connection_blocker blocker() const noexcept
        {
            return connection_blocker(m_Slot);
        }

        void swap(connection& other) noexcept
        {
            std::swap(m_Slot, other.m_Slot);
        }

    protected:
        template<typename, typename>
        friend class signal;

        friend class scoped_connection;

        explicit connection(slot_type&& s) noexcept
            : m_Slot{ std::move(s) }
        {}

        slot_type m_Slot;
    };

    inline void swap(connection& c1, connection& c2)
    {
        c1.swap(c2);
    }
    

    /**
     * An RAII version of connection which disconnects it's slot on destruction.
     */
    class scoped_connection : public connection
    {
    public:
        using base = connection;
        using slot_type = slot_wptr;

        scoped_connection() = default;
        virtual ~scoped_connection() override
        {
            this->disconnect();
        }

        scoped_connection(const base& c) noexcept
            : base(c)
        {}

        scoped_connection(base&& c) noexcept
            : base(std::move(c))
        {}

        scoped_connection(const scoped_connection&) noexcept = delete;

        scoped_connection(scoped_connection&& s) noexcept
            : base(std::move(s.m_Slot))
        {}

        scoped_connection& operator=(const scoped_connection&) noexcept = delete;

        scoped_connection& operator=(scoped_connection&& c) noexcept
        {
            this->disconnect();
            this->m_Slot = std::move(c.m_Slot);
            return *this;
        }

        connection release()
        {
            return connection(std::move(this->m_Slot));
        }

    private:
        explicit scoped_connection(slot_type s) noexcept
            : base(std::move(s))
        {}
    };

    inline void swap(scoped_connection& c1, scoped_connection& c2)
    {
        c1.swap(c2);
    }

    // Default combiner for signals returns an optional value.
    // The combiner just returns the result of the last connected slot.
    // If no slots or functions returns void, a disengaged optional is returned.
    template<typename T>
    class optional_last_value
    {
    public:
        using result_type = opt::optional<T>;

        template<typename InputIterator>
        result_type operator()(InputIterator first, InputIterator last) const
        {
            result_type result;
            while (first != last)
            {
                result_type temp = *first;
                if (temp)   // Skip disengaged results.
                    result = temp;
                ++first;
            }

            return result;
        }
    };

    // Primary template for the signal.
    template<typename Func, typename Combiner = optional_last_value<typename detail::traits::function_traits<Func>::result_type>>
    class signal;

    // Partial specialization taking a callable.
    template<typename R, typename... Args, typename Combiner>
    class signal<R(Args...), Combiner> : detail::signal_base
    {
    public:
        using slot_type = slot<R(Args...)>;
        using slot_ptr_type = std::shared_ptr<slot_type>;
        using list_type = std::vector<slot_ptr_type>;
        using list_iterator = typename list_type::const_iterator;
        using cow_type = detail::cow_ptr<list_type>;
        using mutex_type = std::mutex;
        using lock_type = std::unique_lock<mutex_type>;
        using result_type = typename Combiner::result_type;

        signal()
            : m_Blocked(false)
            , m_Slots(new list_type())
        {}
        ~signal() = default;

        // Not copyable.
        signal(const signal&) = delete;
        // Not copy assignable.
        signal& operator=(const signal&) = delete;

        // Moveable.
        signal(signal&& other) noexcept
            : m_Blocked(other.m_Blocked.load())
        {
            lock_type lock(other.m_SlotMutex);
            m_Slots = std::move(other.m_Slots);
        }

        // Move assignable.
        signal& operator=(signal&& other) noexcept
        {
            lock_type lock1(m_SlotMutex, std::defer_lock);
            lock_type lock2(other.m_SlotMutex, std::defer_lock);
            std::lock(lock1, lock2);

            m_Slots = std::move(other.m_Slots);
            m_Blocked = other.m_Blocked.load();
        }

        // Connect a previously created slot
        connection connect(const slot_type& slot)
        {
            auto s = slot_ptr_type(new slot_type(slot, static_cast<detail::signal_base*>(this)));
            connection c(s);
            add_slot(std::move(s));
            return c;
        }

        // Connect a slot with a callable function object.
        template<typename Func,
            typename = detail::traits::enable_if_t<detail::traits::is_invocable_r<R, detail::traits::remove_cvref_t<Func>, Args...>::value>,
            typename = detail::traits::enable_if_t<!std::is_base_of<detail::slot_base, detail::traits::remove_cvref_t<Func>>::value>>
        connection connect(Func&& f)
        {
            auto s = slot_ptr_type(new slot_type(std::forward<Func>(f), static_cast<detail::signal_base*>(this)));
            connection c(s);
            add_slot(std::move(s));
            return c;
        }

        // Connect a slot with a pointer to member function.
        // or pointer to member data.
        template<typename Func, typename Ptr, 
            typename = detail::traits::enable_if_t<detail::traits::is_invocable_r<R, detail::traits::remove_cvref_t<Func>, Ptr, Args...>::value>,
            typename = detail::traits::enable_if_t<!std::is_base_of<detail::slot_base, detail::traits::remove_cvref_t<Func>>::value>>
        connection connect(Func&& f, Ptr&& p)
        {
            auto s = slot_ptr_type(new slot_type(std::forward<Func>(f), std::forward<Ptr>(p), static_cast<detail::signal_base*>(this)));
            connection c(s);
            add_slot(std::move(s));
            return c;
        }

        // Connect a previously created slot.
        // Returns a scoped_connection.
        scoped_connection connect_scoped(const slot_type& slot)
        {
            return scoped_connection(connect(slot));
        }

        // Connect a slot with a callable function object.
        // Returns a scoped connection.
        template<typename Func, 
            typename = detail::traits::enable_if_t<detail::traits::is_invocable_r<R, detail::traits::remove_cvref_t<Func>, Args...>::value>,
            typename = detail::traits::enable_if_t<!std::is_base_of<detail::slot_base, detail::traits::remove_cvref_t<Func>>::value>>
        scoped_connection connect_scoped(Func&& f)
        {
            return scoped_connection(connect<Func>(std::forward<Func>(f)));
        }

        // Connect a slot with a pointer to member function
        // or pointer to member data.
        // Returns a scoped connection.
        template<typename Func, typename Ptr, 
            typename = detail::traits::enable_if_t<detail::traits::is_invocable_r<R, Func, Ptr, Args...>::value>,
            typename = detail::traits::enable_if_t<!std::is_base_of<detail::slot_base, detail::traits::remove_cvref_t<Func>>::value>>
        scoped_connection connect_scoped(Func&& f, Ptr&& p)
        {
            return scoped_connection(connect<Func>(std::forward<Func>(f), std::forward<Ptr>(p)));
        }

        // Disconnect a slot.
        std::size_t disconnect(const slot_type& slot)
        {
            return erase(slot);
        }

        // Disconnect any slots that are bound to the function object.
        // Returns the number of slots that were disconnected.
        template<typename Func,
            typename = detail::traits::enable_if_t<detail::traits::is_invocable_r<R, Func, Args...>::value>,
            typename = detail::traits::enable_if_t<!std::is_base_of<detail::slot_base, detail::traits::remove_cvref_t<Func>>::value>>
        std::size_t disconnect(Func&& f)
        {
            // Create a temporary slot for comparison.
            auto s = slot<R(Args...)>(std::forward<Func>(f));
            return erase(s);
        }

        // Disconnect any slots that are bound to the function object.
        // Returns the number of slots that were disconnected.
        template<typename Func, typename Ptr,
            typename = detail::traits::enable_if_t<detail::traits::is_invocable_r<R, Func, Ptr, Args...>::value>,
            typename = detail::traits::enable_if_t<!std::is_base_of<detail::slot_base, detail::traits::remove_cvref_t<Func>>::value>>
        std::size_t disconnect(Func&& f, Ptr&& p)
        {
            // Create a temporary slot for comparison.
            auto s = slot<R(Args...)>(std::forward<Func>(f), std::forward<Ptr>(p));
            return erase(s);
        }

        result_type operator()(Args... args) const
        {
            if (m_Blocked) return {};

            auto t = std::tuple<Args...>(std::forward<Args>(args)...);

            // Get a read-only copy of the slots.
            const auto slots_copy = read_slots();
            const auto& slots = slots_copy.read();

            using iterator = detail::slot_iterator<R, list_iterator, Args...>;
            return Combiner()(iterator(slots.begin(), t), iterator(slots.end(), t));
        }

    private:
        template <typename>
        friend class slot;
        
        void add_slot(slot_ptr_type&& s)
        {
            lock_type lock(m_SlotMutex);
            auto& slots = m_Slots.write();

            s->state().index() = slots.size();
            slots.push_back(std::move(s));
        }

        // Remove a slot at the given index.
        virtual void remove_slot(detail::slot_base& slot) override
        {
            lock_type lock(m_SlotMutex);
            auto i = slot.index();
            auto& slots = m_Slots.write();

            if (!slots.empty() && i < slots.size())
            {
                std::swap(slots[i], slots.back());
                slots[i]->index() = i;
                slots.pop_back();
            }
        }

        // Erase all slots that match given slot.
        // @param slot The slot to match for erasure.
        // @returns The number of slots that were actually erased.
        template<typename Func>
        size_t erase(const slot<Func>& slot)
        {
            lock_type lock(m_SlotMutex);
            auto& slots = m_Slots.write();

            std::size_t count = 0;   // The number of slots that were removed.
            std::size_t i = 0;       // Slot index
            while (i < slots.size())
            {
                auto& s = *slots[i];
                if (s == slot)
                {
                    std::swap(slots[i], slots.back());
                    slots[i]->index() = i;
                    slots.pop_back();
                    ++count;
                }
                else
                {
                    ++i;
                }
            }
            return count;
        }

        void clear()
        {
            lock_type lock(m_SlotMutex);
            m_Slots->clear();
        }

        // Get a copy of the slots for reading.
        const cow_type read_slots() const
        {
            lock_type lock(m_SlotMutex);
            return m_Slots;
        }

        mutable mutex_type m_SlotMutex;
        cow_type m_Slots;
        std::atomic_bool m_Blocked;
    };
} // namespace sig