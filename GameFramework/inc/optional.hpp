#pragma once

//          Copyright Jeremiah van Oosten 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//

 /**
  *  @file optional.hpp
  *  @date April 19, 2020
  *  @author Jeremiah van Oosten
  *
  *  @brief C++11 version of C++17's std::optional type based on boost::optional.
  *  @see https://www.boost.org/doc/libs/1_72_0/libs/optional/doc/html/index.html
  */

#include <cassert>          // for assert
#include <stdexcept>        // for std::logic_error
#include <functional>       // for std::reference_wrapper
#include <initializer_list> // for std::initializer_list
#include <memory>           // for std::addressof
#include <string>           // for arguments to opt::bad_optional_access
#include <type_traits>
#include <utility>          // for std::move

  // Check for inline variable support (requires C++17)
#if defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L
#define OPT_INLINE_VAR inline constexpr
#else
#define OPT_INLINE_VAR constexpr
#endif

namespace opt
{
    // Since C++17
    // @see https://en.cppreference.com/w/cpp/utility/optional/bad_optional_access
    class bad_optional_access : public std::logic_error
    {
    public:
        bad_optional_access(const char* what_arg)
            : std::logic_error(what_arg)
        {}

        bad_optional_access(const std::string& what_arg)
            : std::logic_error(what_arg)
        {}
    };

    // Since C++17
    // @see https://en.cppreference.com/w/cpp/utility/optional/nullopt_t
    struct nullopt_t
    {
        struct init_tag {};
        explicit constexpr nullopt_t(init_tag) {}
    };

    // Since C++17
    // @see https://en.cppreference.com/w/cpp/utility/optional/nullopt
    OPT_INLINE_VAR nullopt_t nullopt{ nullopt_t::init_tag() };

    // Forward declare optional template class.
    template<class T> class optional;
    template<class T> class optional<T&>;

    // A tag for in-place initialization of contained value.
    struct in_place_t
    {
        struct init_tag {};
        explicit constexpr in_place_t(init_tag) {}
    };

    OPT_INLINE_VAR in_place_t in_place{ in_place_t::init_tag() };

    // A tag for conditional in-place initialization of contained value.
    struct in_place_if_t
    {
        struct init_tag {};
        explicit constexpr in_place_if_t(init_tag) {}
    };

    OPT_INLINE_VAR in_place_if_t in_place_if{ in_place_if_t::init_tag() };

    namespace detail
    {
        struct init_value_tag {};
        struct optional_tag {};

        namespace traits
        {
            // Since C++14
            // @see https://en.cppreference.com/w/cpp/types/decay
            template< class T >
            using decay_t = typename std::decay<T>::type;

            // Since C++14
            // @see https://en.cppreference.com/w/cpp/types/enable_if
            template< bool B, class T = void >
            using enable_if_t = typename std::enable_if<B, T>::type;

            // Since C++14
            // @see https://en.cppreference.com/w/cpp/types/conditional
            template< bool B, class T, class F >
            using conditional_t = typename std::conditional<B, T, F>::type;

            // definition of metafunciton is_optional_val_init_candidate
            template <typename U>
            struct is_optional_related
                : conditional_t<std::is_base_of<opt::detail::optional_tag, decay_t<U>>::value
                || std::is_same<decay_t<U>, opt::nullopt_t>::value
                || std::is_same<decay_t<U>, in_place_t>::value
                || std::is_same<decay_t<U>, in_place_if_t>::value,
                std::true_type, std::false_type>
            {};

            template <typename T, typename U>
            struct is_optional_constructible : std::is_constructible<T, U>
            {};

            template <typename T, typename U>
            struct is_optional_val_init_candidate
                : conditional_t<!is_optional_related<U>::value&& std::is_convertible<T, U>::value
                , std::true_type, std::false_type>
            {};
        } // namespace traits

        template<typename T>
        class optional_base : public optional_tag
        {
        private:
            using storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
            using this_type = optional_base<T>;

            bool m_initialized;
            storage_type m_storage;

        protected:
            using value_type = T;
            using reference_type = T&;
            using reference_const_type = T const&;
            using rval_reference_type = T&&;
            using reference_type_of_temporary_wrapper = T&&;
            using pointer_type = T*;
            using pointer_const_type = T const*;
            using argument_type = T const&;

            // Creates an optional<T> uninitialized
            optional_base() noexcept
                : m_initialized(false)
                , m_storage{}
            {}

            // Creates an optional<T> uninitialized
            optional_base(opt::nullopt_t) noexcept
                : m_initialized(false)
                , m_storage{}
            {}

            // Creates an optional<T> initialized with 'val'.
            // Can throw if T::T(T const&) does
            optional_base(init_value_tag, argument_type val)
                : m_initialized(false)
            {
                construct(val);
            }

            // move-construct an optional<T> initialized from an rvalue-ref to 'val'.
            // Can throw if T::T(T&&) does
            optional_base(init_value_tag, rval_reference_type val)
                : m_initialized(false)
            {
                construct(std::move(val));
            }

            // Creates an optional<T> initialized with 'val' IFF cond is true, 
            // otherwise creates an uninitialized optional<T>.
            // Can throw if T::T(T const&) does
            optional_base(bool cond, argument_type val)
                : m_initialized(false)
            {
                if (cond)
                    construct(val);
            }

            // Creates an optional<T> initialized with 'move(val)' IFF cond is true, 
            // otherwise creates an uninitialized optional<T>.
            // Can throw if T::T(T &&) does
            optional_base(bool cond, rval_reference_type val)
                : m_initialized(false)
            {
                if (cond)
                    construct(std::move(val));
            }

            // Creates a deep copy of another optional<T>
            // Can throw if T::T(T const&) does
            optional_base(optional_base const& rhs)
                : m_initialized(false)
            {
                if (rhs.is_initialized())
                    construct(rhs.get_impl());
            }

            // Creates a deep move of another optional<T>
            // Can throw if T::T(T&&) does
            optional_base(optional_base&& rhs)
                noexcept((std::is_nothrow_move_constructible<T>::value))
                : m_initialized(false)
            {
                if (rhs.is_initialized())
                    construct(std::move(rhs.get_impl()));
            }

            // This is used for both converting and in-place constructions.
            // Derived classes use the 'tag' to select the appropriate
            // implementation (the correct 'construct()' overload)
            template<class Expr>
            explicit optional_base(Expr&& expr, void const* tag)
                : m_initialized(false)
            {
                construct(std::forward<Expr>(expr), tag);
            }

            optional_base& operator=(optional_base const& rhs)
            {
                this->assign(rhs);

                return *this;
            }

            optional_base& operator=(optional_base&& rhs)
                noexcept((std::is_nothrow_move_constructible<T>::value
                    && std::is_nothrow_move_assignable<T>::value))
            {
                // This is the original cast from the boost C++ library.
                // Note: Shouldn't this be this->assign(std::move(rhs)) ?
                //this->assign(std::move(rhs));
                this->assign(static_cast<optional_base&&>(rhs));

                return *this;
            }

            // No-throw (assuming T::~T() doesn't)
            ~optional_base()
            {
                destroy();
            }

            // Assigns from another optional<T> (deep-copies the rhs value)
            void assign(optional_base const& rhs)
            {
                if (is_initialized())
                {
                    if (rhs.is_initialized())
                        assign_value(rhs.get_impl());
                    else
                        destroy();
                }
                else
                {
                    if (rhs.is_initialized())
                        construct(rhs.get_impl());
                }
            }

            // Assigns from another optional<T> (deep-moves the rhs value)
            void assign(optional_base&& rhs)
            {
                if (is_initialized())
                {
                    if (rhs.is_initialized())
                        assign_value(std::move(rhs.get_impl()));
                    else
                        destroy();
                }
                else
                {
                    if (rhs.is_initialized())
                        construct(std::move(rhs.get_impl()));
                }
            }

            // Assigns from another _convertible_ optional<U> (deep-copies the rhs value)
            template<class U>
            void assign(opt::optional<U> const& rhs)
            {
                if (is_initialized())
                {
                    if (rhs.is_initialized())
                        assign_value(rhs.get());
                    else
                        destroy();
                }
                else
                {
                    if (rhs.is_initialized())
                        construct(rhs.get());
                }
            }

            // move-assigns from another _convertible_ optional<U> (deep-moves from the rhs value)
            template<class U>
            void assign(optional<U>&& rhs)
            {
                using ref_type = typename optional<U>::rval_reference_type;

                if (is_initialized())
                {
                    if (rhs.is_initialized())
                        assign_value(static_cast<ref_type>(rhs.get()));
                    else destroy();
                }
                else
                {
                    if (rhs.is_initialized())
                        construct(static_cast<ref_type>(rhs.get()));
                }
            }

            // Assigns from a T (deep-copies the rhs value)
            void assign(argument_type val)
            {
                if (is_initialized())
                    assign_value(val);
                else
                    construct(val);
            }

            // Assigns from a T (deep-moves the rhs value)
            void assign(rval_reference_type val)
            {
                if (is_initialized())
                    assign_value(std::move(val));
                else
                    construct(std::move(val));
            }

            // Assigns from "none", destroying the current value, if any, leaving this UNINITIALIZED
            // No-throw (assuming T::~T() doesn't)
            void assign(opt::nullopt_t) noexcept
            {
                destroy();
            }

        public:

            // Destroys the current value, if any, leaving this UNINITIALIZED
            // No-throw
            void reset() noexcept
            {
                destroy();
            }

            // Returns a pointer to the value if this is initialized, otherwise,
            // returns NULL.
            // No-throw
            pointer_const_type get_ptr() const
            {
                return m_initialized ? get_ptr_impl() : nullptr;
            }

            pointer_type get_ptr()
            {
                return m_initialized ? get_ptr_impl() : nullptr;
            }

            bool is_initialized() const noexcept
            {
                return m_initialized;
            }

        protected:

            void construct(argument_type val)
            {
                ::new(&m_storage) value_type(val);
                m_initialized = true;
            }

            void construct(rval_reference_type val)
            {
                ::new(&m_storage) value_type(std::move(val));
                m_initialized = true;
            }

            // Constructs in-place
            // upon exception *this is always uninitialized
            template<class... Args>
            void construct(in_place_t, Args&&... args)
            {
                ::new (&m_storage) value_type(std::forward<Args>(args)...);
                m_initialized = true;
            }

            template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            void construct(in_place_t, std::initializer_list<U> il, Args&&... args)
            {
                ::new (&m_storage) value_type(il, std::forward<Args>(args)...);
                m_initialized = true;
            }

            template<class... Args>
            void emplace_assign(Args&&... args)
            {
                destroy();
                construct(in_place, std::forward<Args>(args)...);
            }

            template<class... Args>
            explicit optional_base(in_place_t, Args&&... args)
                : m_initialized(false)
            {
                construct(in_place, std::forward<Args>(args)...);
            }

            template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            explicit optional_base(in_place_t, std::initializer_list<U> il, Args&&... args)
                : m_initialized(false)
            {
                construct(in_place, il, std::forward<Args>(args)...);
            }

            template<class... Args>
            explicit optional_base(in_place_if_t, bool cond, Args&&... args)
                : m_initialized(false)
                , m_storage{}
            {
                if (cond)
                    construct(in_place, std::forward<Args>(args)...);
            }

            template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            explicit optional_base(in_place_if_t, bool cond, std::initializer_list<U> il, Args&&... args)
                : m_initialized(false)
            {
                if (cond)
                    construct(in_place, il, std::forward<Args>(args)...);
            }

            //// Constructs using any expression implicitly convertible to the single argument
            //// of a one-argument T constructor.
            //// Converting constructions of optional<T> from optional<U> uses this function with
            //// 'Expr' being of type 'U' and relying on a converting constructor of T from U.
            //template<class Expr>
            //void construct(Expr&& expr, void const*)
            //{
            //    new (&m_storage) value_type(std::forward<Expr>(expr));
            //    m_initialized = true;
            //}

            // Assigns using a form any expression implicitly convertible to the single argument
            // of a T's assignment operator.
            // Converting assignments of optional<T> from optional<U> uses this function with
            // 'Expr' being of type 'U' and relying on a converting assignment of T from U.
            //template<class Expr>
            //void assign_expr_to_initialized(Expr&& expr, void const*)
            //{
            //    assign_value(std::forward<Expr>(expr));
            //}

            void assign_value(argument_type val)
            {
                get_impl() = val;
            }

            void assign_value(rval_reference_type val)
            {
                // Note: Shouldn't this be?:
                // get_impl() = std::move(val);
                get_impl() = static_cast<rval_reference_type>(val);
            }

            void destroy()
            {
                if (m_initialized)
                    destroy_impl();
            }

            reference_const_type get_impl() const
            {
                return reinterpret_cast<reference_const_type>(m_storage);
            }

            reference_type get_impl()
            {
                return reinterpret_cast<reference_type>(m_storage);
            }

            pointer_const_type get_ptr_impl() const
            {
                return reinterpret_cast<pointer_const_type>(&m_storage);
            }

            pointer_type get_ptr_impl()
            {
                return reinterpret_cast<pointer_type>(&m_storage);
            }

        private:
            void destroy_impl()
            {
                get_impl().T::~T();
                m_initialized = false;
            }
        };

        // Trivilally-copyable version of the storage
        template<class T>
        class tc_optional_base : public optional_tag
        {
        private:
            using this_type = tc_optional_base<T>;

            bool m_initialized;
            T    m_storage;

        protected:
            using value_type = T;
            using reference_type = T&;
            using reference_const_type = T const&;
            using rval_reference_type = T&&;
            using reference_type_of_temporary_wrapper = T&&;
            using pointer_type = T*;
            using pointer_const_type = T const*;
            using argument_type = T const&;

            tc_optional_base() noexcept
                : m_initialized(false)
                , m_storage{}
            {}

            tc_optional_base(opt::nullopt_t) noexcept
                : m_initialized(false)
                , m_storage{}
            {}

            tc_optional_base(init_value_tag, argument_type val)
                : m_initialized(true)
                , m_storage{val}
            {}

            tc_optional_base(bool cond, argument_type val)
                : m_initialized(cond)
                , m_storage{val}
            {}

            //template<class Expr>
            //explicit tc_optional_base(Expr&& expr, void const* tag)
            //    : m_initialized(false)
            //{
            //    construct(std::forward<Expr>(expr), tag);
            //}

            // Assigns from another optional<T> (deep-copies the rhs value)
            void assign(tc_optional_base const& rhs)
            {
                *this = rhs;
            }

            // Assigns from another _convertible_ optional<U> (deep-copies the rhs value)
            template<class U>
            void assign(optional<U> const& rhs)
            {
                if (rhs.is_initialized())
                    m_storage = rhs.get();

                m_initialized = rhs.is_initialized();
            }

            // move-assigns from another _convertible_ optional<U> (deep-moves from the rhs value)
            template<class U>
            void assign(optional<U>&& rhs)
            {
                using ref_type = typename optional<U>::rval_reference_type;

                if (rhs.is_initialized())
                    m_storage = static_cast<ref_type>(rhs.get());

                m_initialized = rhs.is_initialized();
            }

            void assign(argument_type val)
            {
                construct(val);
            }

            void assign(opt::nullopt_t)
            {
                destroy();
            }

        public:

            // Destroys the current value, if any, leaving this UNINITIALIZED
            // No-throw (assuming T::~T() doesn't)
            void reset() noexcept
            {
                destroy();
            }

            // Returns a pointer to the value if this is initialized, otherwise,
            // returns NULL.
            // No-throw
            pointer_const_type get_ptr() const noexcept
            {
                return m_initialized ? get_ptr_impl() : nullptr;
            }

            pointer_type get_ptr() noexcept
            {
                return m_initialized ? get_ptr_impl() : nullptr;
            }

            bool is_initialized() const noexcept
            {
                return m_initialized;
            }

        protected:
            void construct(argument_type val)
            {
                m_storage = val;
                m_initialized = true;
            }

            // Constructs in-place
            // upon exception *this is always uninitialized
            template<class... Args>
            void construct(in_place_t, Args&&... args)
            {
                m_storage = value_type(std::forward<Args>(args)...);
                m_initialized = true;
            }

            template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            void construct(in_place_t, std::initializer_list<U> il, Args&&... args)
            {
                m_storage = value_type(il, std::forward<Args>(args)...);
                m_initialized = true;
            }

            template<class... Args>
            void emplace_assign(Args&&... args)
            {
                construct(in_place, std::forward<Args>(args)...);
            }

            template<class... Args>
            explicit tc_optional_base(in_place_t, Args&&... args)
                : m_initialized(false)
            {
                construct(in_place, std::forward<Args>(args)...);
            }

            template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            explicit tc_optional_base(in_place_t, std::initializer_list<U> il, Args&&... args)
                : m_initialized(false)
            {
                construct(in_place, il, std::forward<Args>(args)...);
            }

            template<class... Args>
            explicit tc_optional_base(in_place_if_t, bool cond, Args&&... args)
                : m_initialized(false)
                , m_storage{}
            {
                if (cond)
                    construct(in_place, std::forward<Args>(args)...);
            }

            template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
            explicit tc_optional_base(in_place_if_t, bool cond, std::initializer_list<U> il, Args&&... args)
                : m_initialized(false)
            {
                if (cond)
                    construct(in_place, il, std::forward<Args>(args)...);
            }

            // Constructs using any expression implicitly convertible to the single argument
            // of a one-argument T constructor.
            // Converting constructions of optional<T> from optional<U> uses this function with
            // 'Expr' being of type 'U' and relying on a converting constructor of T from U.
            //template<class Expr>
            //void construct(Expr&& expr, void const*)
            //{
            //    m_storage = value_type(std::forward<Expr>(expr));
            //    m_initialized = true;
            //}

            // Assigns using a form any expression implicitly convertible to the single argument
            // of a T's assignment operator.
            // Converting assignments of optional<T> from optional<U> uses this function with
            // 'Expr' being of type 'U' and relying on a converting assignment of T from U.
            //template<class Expr>
            //void assign_expr_to_initialized(Expr&& expr, void const*)
            //{
            //    assign_value(std::forward<Expr>(expr));
            //}

            void assign_value(argument_type val)
            {
                m_storage = val;
            }

            void assign_value(rval_reference_type val)
            {
                m_storage = static_cast<rval_reference_type>(val);
            }

            reference_const_type get_impl() const
            {
                return m_storage;
            }

            reference_type get_impl()
            {
                return m_storage;
            }

            pointer_const_type get_ptr_impl() const
            {
                return std::addressof(m_storage);
            }

            pointer_type get_ptr_impl()
            {
                return std::addressof(m_storage);
            }

            void destroy()
            {
                m_initialized = false;
            }
        };

        namespace config
        {
            template <typename T>
            struct optional_uses_direct_storage_for
                : traits::conditional_t<std::is_scalar<T>::value && !std::is_const<T>::value && !std::is_volatile<T>::value
                , std::true_type, std::false_type>
            {};

        } // namespace config

        //template<typename T>
        //using optional_base_type = traits::conditional_t< config::optional_uses_direct_storage_for<T>::value, tc_optional_base<T>, optional_base<T>>;

        template<typename T>
        using optional_base_type = traits::conditional_t< config::optional_uses_direct_storage_for<T>::value, tc_optional_base<T>, optional_base<T>>;

    } // namespace detail

    template<class T>
    class optional : public detail::optional_base_type<T>
    {
        static_assert(!std::is_same<detail::traits::decay_t<T>, nullopt_t>::value, "Cannot create optional<nullopt_t>");
        static_assert(!std::is_same<detail::traits::decay_t<T>, detail::optional_tag>::value, "Cannot create optional<optional_tag>");
        static_assert(!std::is_same<detail::traits::decay_t<T>, in_place_t>::value, "Cannot create optional<in_place_t>");
        static_assert(!std::is_same<detail::traits::decay_t<T>, in_place_if_t>::value, "Cannot create optional<in_place_if_t>");

    private:
        using base = detail::optional_base_type<T>;

    public:
        using this_type = optional<T>;
        using value_type = typename base::value_type;
        using reference_type = typename base::reference_type;
        using reference_const_type = typename base::reference_const_type;
        using rval_reference_type = typename base::rval_reference_type;
        using reference_type_of_temporary_wrapper = typename base::reference_type_of_temporary_wrapper;
        using pointer_type = typename base::pointer_type;
        using pointer_const_type = typename base::pointer_const_type;
        using argument_type = typename base::argument_type;

        // Creates an optional<T> uninitialized.
        // No-throw
        optional() noexcept
            : base()
        {}

        // Creates an optional<T> uninitialized.
        // No-throw
        optional(nullopt_t none) noexcept
            : base(none)
        {}

        // Creates an optional<T> initialized with 'val'.
        // Can throw if T::T(T const&) does
        optional(argument_type val)
            : base(detail::init_value_tag(), val)
        {}

        // Creates an optional<T> initialized with 'move(val)'.
        // Can throw if T::T(T &&) does
        optional(rval_reference_type val)
            : base(detail::init_value_tag(), std::forward<T>(val))
        {}

        /// Creates an optional<T> initialized with 'val' IFF cond is true, otherwise creates an uninitialized optional.
        // Can throw if T::T(T &&) does
        optional(bool cond, rval_reference_type val)
            : base(cond, std::forward<T>(val))
        {}

        // Creates a deep copy of another convertible optional<U>
        // Requires a valid conversion from U to T.
        // Can throw if T::T(U const&) does
        template<class U, typename = detail::traits::enable_if_t<detail::traits::is_optional_constructible<T, U const&>::value>>
        explicit optional(optional<U> const& rhs)
            : base()
        {
            if (rhs.is_initialized())
                this->construct(rhs.get());
        }

        // Creates a deep move of another convertible optional<U>
        // Requires a valid conversion from U to T.
        // Can throw if T::T(U&&) does
        template<class U, typename = detail::traits::enable_if_t<detail::traits::is_optional_constructible<T, U>::value>>
        explicit optional(optional<U>&& rhs)
            : base()
        {
            if (rhs.is_initialized())
                this->construct(std::move(rhs.get()));
        }

        // Creates a deep copy of another optional<T>
        // Can throw if T::T(T const&) does
        optional(optional const&) = default;

        // Creates a deep move of another optional<T>
        // Can throw if T::T(T&&) does
        optional(optional&& rhs) = default;

        //  On old MSVC compilers the implicitly declared dtor is not called
        ~optional() {}

        // Copy-assigns from another convertible optional<U> (converts && deep-copies the rhs value)
        // Requires a valid conversion from U to T.
        // Basic Guarantee: If T::T( U const& ) throws, this is left UNINITIALIZED
        template<class U>
        optional& operator=(optional<U> const& rhs)
        {
            this->assign(rhs);
            return *this;
        }

        // Move-assigns from another convertible optional<U> (converts && deep-moves the rhs value)
        // Requires a valid conversion from U to T.
        // Basic Guarantee: If T::T( U && ) throws, this is left UNINITIALIZED
        template<class U>
        optional& operator=(optional<U>&& rhs)
        {
            this->assign(std::move(rhs));
            return *this;
        }

        // Assigns from another optional<T> (deep-copies the rhs value)
        // Basic Guarantee: If T::T( T const& ) throws, this is left UNINITIALIZED
        //  (NOTE: On BCB, this operator is not actually called and left is left UNMODIFIED in case of a throw)
        optional& operator=(optional const& rhs) = default;

        // Assigns from another optional<T> (deep-moves the rhs value)
        optional& operator=(optional&&) = default;

        // Assigns from a T (deep-moves/copies the rhs value)
        template <typename U>
        detail::traits::enable_if_t<std::is_same<T, detail::traits::decay_t<U>>::value, optional&>
            operator=(U&& val)
        {
            this->assign(std::forward<U>(val));
            return *this;
        }

        // Assigns from a "none"
        // Which destroys the current value, if any, leaving this UNINITIALIZED
        // No-throw (assuming T::~T() doesn't)
        optional& operator=(nullopt_t none) noexcept
        {
            this->assign(none);
            return *this;
        }

        // Constructs in-place
        // upon exception *this is always uninitialized
        template<class... Args>
        void emplace(Args&&... args)
        {
            this->emplace_assign(std::forward<Args>(args)...);
        }

        template<class U, class... Args>
        void emplace(std::initializer_list<U> il, Args&&... args)
        {
            this->emplace_assign(il, std::forward<Args>(args)...);
        }

        template<class... Args>
        explicit optional(in_place_t, Args&&... args)
            : base(in_place, std::forward<Args>(args)...)
        {}

        template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
        explicit optional(in_place_t, std::initializer_list<U> il, Args&&... args)
            : base(in_place, il, std::forward<Args>(args)...)
        {}

        template<class... Args>
        explicit optional(in_place_if_t, bool cond, Args&&... args)
            : base(in_place_if, cond, std::forward<Args>(args)...)
        {}

        template<class U, class... Args, typename = detail::traits::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>>
        explicit optional(in_place_if_t, bool cond, std::initializer_list<U> il, Args&&... args)
            : base(in_place_if, cond, il, std::forward<Args>(args)...)
        {}

        void swap(optional& rhs) noexcept((std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value))
        {
            // allow for Koenig lookup
            std::swap(*this, rhs);
        }

        // Returns a reference to the value if this is initialized, otherwise,
        // the behavior is UNDEFINED
        // No-throw
        reference_const_type get() const
        {
            assert(this->is_initialized());
            return this->get_impl();
        }

        reference_type get()
        {
            assert(this->is_initialized());
            return this->get_impl();
        }

        // Returns a copy of the value if this is initialized, 'v' otherwise
        reference_const_type get_value_or(reference_const_type v) const
        {
            return this->is_initialized() ? get() : v;
        }

        reference_type get_value_or(reference_type v)
        {
            return this->is_initialized() ? get() : v;
        }

        // Returns a pointer to the value if this is initialized, otherwise,
        // the behaviour is UNDEFINED
        // No-throw
        pointer_const_type operator->() const
        {
            assert(this->is_initialized());
            return this->get_ptr_impl();
        }
        pointer_type operator->()
        {
            assert(this->is_initialized());
            return this->get_ptr_impl();
        }

        // Returns a reference to the value if this is initialized, otherwise,
        // the behaviour is UNDEFINED
        // No-throw
        reference_const_type operator*() const&
        {
            return this->get();
        }

        reference_type operator*()&
        {
            return this->get();
        }

        reference_type_of_temporary_wrapper operator*()&&
        {
            return std::move(this->get());
        }

        reference_const_type value() const&
        {
            if (this->is_initialized())
                return this->get();
            else
                throw bad_optional_access("Attempted to retrieve the value of a disengaged optional.");
        }

        reference_type value()&
        {
            if (this->is_initialized())
                return this->get();
            else
                throw bad_optional_access("Attempted to retrieve the value of a disengaged optional.");
        }

        reference_type_of_temporary_wrapper value()&&
        {
            if (this->is_initialized())
                return std::move(this->get());
            else
                throw bad_optional_access("Attempted to retrieve the value of a disengaged optional.");
        }

        template <class U>
        value_type value_or(U&& v) const&
        {
            if (this->is_initialized())
                return get();
            else
                return std::forward<U>(v);
        }

        template <class U>
        value_type value_or(U&& v)&&
        {
            if (this->is_initialized())
                return std::move(get());
            else
                return std::forward<U>(v);
        }

        // Explicit conversion to bool.
        explicit constexpr operator bool() const noexcept
        {
            return this->is_initialized();
        }

        constexpr bool has_value() const noexcept
        {
            return this->is_initialized();
        }

    };

    // Optional that takes a reference type.
    template <class T>
    class optional<T&> : public detail::optional_tag
    {
        static_assert(!std::is_same<T, nullopt_t>::value, "Cannot create optional<nullopt_t>");
        static_assert(!std::is_same<T, detail::optional_tag>::value, "Cannot create optional<optional_tag>");
        static_assert(!std::is_same<T, in_place_t>::value, "Cannot create optional<in_place_init_t>");
        static_assert(!std::is_same<T, in_place_if_t>::value, "Cannot create optional<in_place_init_if_t>");

        T* ref;

    public:
        using value_type = T&;
        using reference_type = T&;
        using reference_const_type = T&;
        using rval_reference_type = T&;
        using pointer_type = T*;
        using pointer_const_type = T*;

        constexpr optional() noexcept : ref(nullptr) {}
        constexpr optional(nullopt_t) noexcept : ref(nullptr) {}
        constexpr optional(T& v) noexcept : ref(std::addressof(v)) {}
        constexpr optional(const optional& rhs) noexcept : ref(rhs.ref) {}
        explicit constexpr optional(in_place_t, T& v) noexcept : ref(std::addressof(v)) {}

        optional(T&&) = delete;
        explicit optional(in_place_t, T&&) = delete;

        ~optional() = default;

        optional& operator=(nullopt_t) noexcept {
            ref = nullptr;
            return *this;
        }

        template <typename U>
        auto operator=(U&& rhs) noexcept
            -> detail::traits::enable_if_t<std::is_same<detail::traits::decay_t<U>, optional<T&>>::value, optional&>
        {
            ref = rhs.ref;
            return *this;
        }

        template <typename U>
        auto operator=(U&& rhs) noexcept
            ->detail::traits::enable_if_t<!std::is_same<detail::traits::decay_t<U>, optional<T&>>::value, optional&>
            = delete;

        void emplace(T& v) noexcept
        {
            ref = std::addressof(v);
        }

        void emplace(T&&) = delete;


        void swap(optional<T&>& rhs) noexcept
        {
            std::swap(ref, rhs.ref);
        }

        constexpr T* operator->() const
        {
            return ref ? ref : (assert(ref), ref);
        }

        constexpr T& operator*() const
        {
            return ref ? *ref : (assert(ref), *ref);
        }

        constexpr T& value() const {
            return ref ? *ref : (throw bad_optional_access("Attempted to retrieve the value of a disengaged optional."), *ref);
        }

        explicit constexpr operator bool() const noexcept 
        {
            return ref != nullptr;
        }

        constexpr bool has_value() const noexcept {
            return ref != nullptr;
        }

        template <class V>
        constexpr detail::traits::decay_t<T> value_or(V&& v) const
        {
            return ref ? *ref : std::forward<V>(v);
        }

        void reset() noexcept { ref = nullptr; }
    };

    // Specialization for void types.
    // Void optionals are always disengaged and you should never try to retrieve
    // the value of a void optional.
    template<>
    class optional<void> : public detail::optional_tag
    {
    public:
        using value_type = void;
        using reference_type = void;
        using reference_const_type = void;
        using rval_reference_type = void;
        using pointer_type = void*;
        using pointer_const_type = void*;

        constexpr optional() noexcept = default;
        constexpr optional(nullopt_t) noexcept {}
        constexpr optional(const optional& rhs) noexcept = default;

        ~optional() = default;

        optional& operator=(nullopt_t) noexcept {
            return *this;
        }

        template <typename U>
        optional<void>& operator=(U&& rhs) noexcept
        {
            return *this;
        }

        template<typename U>
        void emplace(U&) noexcept
        {}

        template<typename U>
        void emplace(U&&) noexcept
        {}

        void swap(optional<void>& rhs) noexcept
        {}

        void operator->() const
        {
            assert(!"Cannot dereference void optional");
        }

        void operator*() const
        {
            assert(!"Cannot dereference void optional");
        }

        void value() const {
            throw bad_optional_access("Attempted to retrieve the value of a void optional.");
        }

        explicit constexpr operator bool() const noexcept
        {
            return false;
        }

        constexpr bool has_value() const noexcept {
            return false;
        }

        template <class V>
        constexpr auto value_or(V&& v) const
        -> decltype(std::forward<V>(v))
        {
            return std::forward<V>(v);
        }

        void reset() noexcept {}
    };

    template<class T>
    class optional<T&&>
    {
        static_assert(sizeof(T) == 0, "Optional rvalue references are illegal.");
    };

    // Relational operators
    template <class T>
    constexpr bool operator==(const optional<T>& x, const optional<T>& y)
    {
        return bool(x) && bool(y) ? *x == *y : bool(x) == bool(y);
    }

    template <class T>
    constexpr bool operator!=(const optional<T>& x, const optional<T>& y)
    {
        return !(x == y);
    }

    template <class T>
    constexpr bool operator<(const optional<T>& x, const optional<T>& y)
    {
        return (!y) ? false : (!x) ? true : *x < *y;
    }

    template <class T>
    constexpr bool operator>(const optional<T>& x, const optional<T>& y)
    {
        return (y < x);
    }

    template <class T>
    constexpr bool operator<=(const optional<T>& x, const optional<T>& y)
    {
        return !(y < x);
    }

    template <class T>
    constexpr bool operator>=(const optional<T>& x, const optional<T>& y)
    {
        return !(x < y);
    }

    template <class T>
    constexpr bool operator==(const optional<T>& x, nullopt_t) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator==(nullopt_t, const optional<T>& x) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator!=(const optional<T>& x, nullopt_t) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator!=(nullopt_t, const optional<T>& x) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator<(const optional<T>&, nullopt_t) noexcept
    {
        return false;
    }

    template <class T>
    constexpr bool operator<(nullopt_t, const optional<T>& x) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator<=(const optional<T>& x, nullopt_t) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept
    {
        return true;
    }

    template <class T>
    constexpr bool operator>(const optional<T>& x, nullopt_t) noexcept
    {
        return bool(x);
    }

    template <class T>
    constexpr bool operator>(nullopt_t, const optional<T>&) noexcept
    {
        return false;
    }

    template <class T>
    constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept
    {
        return true;
    }

    template <class T>
    constexpr bool operator>=(nullopt_t, const optional<T>& x) noexcept
    {
        return (!x);
    }

    template <class T>
    constexpr bool operator==(const optional<T>& x, const T& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator==(const T& v, const optional<T>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!=(const optional<T>& x, const T& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!=(const T& v, const optional<T>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator<(const optional<T>& x, const T& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator>(const T& v, const optional<T>& x)
    {
        return bool(x) ? v > * x : true;
    }

    template <class T>
    constexpr bool operator>(const optional<T>& x, const T& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator<(const T& v, const optional<T>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>=(const optional<T>& x, const T& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<=(const T& v, const optional<T>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<=(const optional<T>& x, const T& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>=(const T& v, const optional<T>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    // Comparison of optional<T&> with T
    template <class T>
    constexpr bool operator==(const optional<T&>& x, const T& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator==(const T& v, const optional<T&>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!=(const optional<T&>& x, const T& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!=(const T& v, const optional<T&>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator<(const optional<T&>& x, const T& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator>(const T& v, const optional<T&>& x)
    {
        return bool(x) ? v > * x : true;
    }

    template <class T>
    constexpr bool operator>(const optional<T&>& x, const T& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator<(const T& v, const optional<T&>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>=(const optional<T&>& x, const T& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<=(const T& v, const optional<T&>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<=(const optional<T&>& x, const T& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>=(const T& v, const optional<T&>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    // Comparison of optional<T const&> with T
    template <class T>
    constexpr bool operator==(const optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x == v : false;
    }

    template <class T>
    constexpr bool operator==(const T& v, const optional<const T&>& x)
    {
        return bool(x) ? v == *x : false;
    }

    template <class T>
    constexpr bool operator!=(const optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x != v : true;
    }

    template <class T>
    constexpr bool operator!=(const T& v, const optional<const T&>& x)
    {
        return bool(x) ? v != *x : true;
    }

    template <class T>
    constexpr bool operator<(const optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x < v : true;
    }

    template <class T>
    constexpr bool operator>(const T& v, const optional<const T&>& x)
    {
        return bool(x) ? v > * x : true;
    }

    template <class T>
    constexpr bool operator>(const optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x > v : false;
    }

    template <class T>
    constexpr bool operator<(const T& v, const optional<const T&>& x)
    {
        return bool(x) ? v < *x : false;
    }

    template <class T>
    constexpr bool operator>=(const optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x >= v : false;
    }

    template <class T>
    constexpr bool operator<=(const T& v, const optional<const T&>& x)
    {
        return bool(x) ? v <= *x : false;
    }

    template <class T>
    constexpr bool operator<=(const optional<const T&>& x, const T& v)
    {
        return bool(x) ? *x <= v : true;
    }

    template <class T>
    constexpr bool operator>=(const T& v, const optional<const T&>& x)
    {
        return bool(x) ? v >= *x : true;
    }

    template<class T>
    constexpr optional<detail::traits::decay_t<T>> make_optional(T&& v)
    {
        return optional<detail::traits::decay_t<T>>(std::forward<T>(v));
    }

    template<class T>
    constexpr optional<T&> make_optional(std::reference_wrapper<T> v)
    {
        return optional<T&>(v.get());
    }

    template<class T>
    constexpr optional<detail::traits::decay_t<T>> make_optional(bool cond, T&& v)
    {
        return optional<detail::traits::decay_t<T>>(in_place_if, cond, std::forward<T>(v));
    }

    template<class T, typename... Args>
    constexpr optional<detail::traits::decay_t<T>> make_optional(Args&&... args)
    {
        return optional<detail::traits::decay_t<T>>(in_place, std::forward<Args>(args)...);
    }

    template<class T, typename... Args>
    constexpr optional<detail::traits::decay_t<T>> make_optional(bool cond, Args&&... args)
    {
        return optional<detail::traits::decay_t<T>>(in_place_if, cond, std::forward<Args>(args)...);
    }

    template<class T>
    constexpr typename optional<T>::reference_const_type get(optional<T> const& opt)
    {
        return opt.get();
    }

    template<class T>
    constexpr typename optional<T>::reference_type get(optional<T>& opt)
    {
        return opt.get();
    }

    template<class T>
    constexpr typename optional<T>::pointer_const_type get(optional<T> const* opt)
    {
        return opt->get_ptr();
    }

    template<class T>
    constexpr typename optional<T>::pointer_type get(optional<T>* opt)
    {
        return opt->get_ptr();
    }

    template<class T>
    constexpr typename optional<T>::reference_const_type
        get_optional_value_or(optional<T> const& opt, typename optional<T>::reference_const_type v)
    {
        return opt.get_value_or(v);
    }

    template<class T>
    constexpr typename optional<T>::reference_type
        get_optional_value_or(optional<T>& opt, typename optional<T>::reference_type v)
    {
        return opt.get_value_or(v);
    }

    // Returns a pointer to the value if this is initialized, otherwise, returns NULL.
    // No-throw
    template<class T>
    constexpr typename optional<T>::pointer_const_type get_pointer(optional<T> const& opt)
    {
        return opt.get_ptr();
    }

    template<class T>
    constexpr typename optional<T>::pointer_type get_pointer(optional<T>& opt)
    {
        return opt.get_ptr();
    }
} // namespace opt

namespace std
{
    // Specialization for optional
    template<class T>
    void swap(opt::optional<T>& x, opt::optional<T>& y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }
}