#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file ThreadSafeQueue.h
  *  @date January 7, 2016
  *  @author Jeremiah
  *
  *  @brief Thread safe queue.
  */

#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue();
    ThreadSafeQueue(const ThreadSafeQueue& copy);

    /**
     * Push a value into the back of the queue.
     */
    void Push(T value);

    /**
     * Try to pop a value from the front of the queue.
     * @returns false if the queue is empty.
     */
    bool TryPop(T& value);

    /**
     * Check to see if there are any items in the queue.
     */
    bool Empty() const;

    /**
     * Retrieve the number of items in the queue.
     */
    size_t Size() const;

private:
    std::queue<T> m_Queue;
    mutable std::mutex m_Mutex;
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue()
{}

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue<T>& copy)
{
    std::lock_guard<std::mutex> lock(copy.m_Mutex);
    m_Queue = other.m_Queue;
}

template<typename T>
void ThreadSafeQueue<T>::Push(T value)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Queue.push(std::move(value));
}

template<typename T>
bool ThreadSafeQueue<T>::TryPop(T& value)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Queue.empty())
        return false;

    value = m_Queue.front();
    m_Queue.pop();

    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::Empty() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Queue.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::Size() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Queue.size();
}