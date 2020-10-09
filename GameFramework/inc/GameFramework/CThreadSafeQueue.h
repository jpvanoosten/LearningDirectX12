//
//	The MIT License
//
//	Copyright (c) 2010 James E Beveridge
//
//	Permission is hereby granted, free of charge, to any person obtaining a
// copy 	of this software and associated documentation files (the
// "Software"), to deal 	in the Software without restriction, including
// without limitation the rights 	to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell 	copies of the Software, and to permit
// persons to whom the
// Software is 	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included
// in 	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR 	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE 	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, 	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN 	THE SOFTWARE.

//	This sample code is for my blog entry titled, "Understanding
// ReadDirectoryChangesW"
//	http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html
//	See ReadMe.txt for overview information.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <list>
#include <mutex>

template<typename C>
class CThreadSafeQueue : protected std::list<C>
{
public:
    CThreadSafeQueue( int nMaxCount )
    {
        m_bOverflow = false;

        m_hSemaphore = ::CreateSemaphore( NULL,       // no security attributes
                                          0,          // initial count
                                          nMaxCount,  // max count
                                          NULL );     // anonymous
    }

    ~CThreadSafeQueue()
    {
        ::CloseHandle( m_hSemaphore );
        m_hSemaphore = NULL;
    }

    void push( const C& c )
    {
        //		CComCritSecLock<CComAutoCriticalSection> lock( m_Crit,
        // true );
        MutexLock lock( m_Crit );

        __super::push_back( c );

        lock.unlock();

        if ( !::ReleaseSemaphore( m_hSemaphore, 1, NULL ) )
        {
            // If the semaphore is full, then take back the entry.
            __super::pop_back();
            if ( ::GetLastError() == ERROR_TOO_MANY_POSTS )
            {
                m_bOverflow = true;
            }
        }
    }

    bool pop( C& c )
    {
        //		CComCritSecLock<CComAutoCriticalSection> lock( m_Crit,
        // true );
        MutexLock lock( m_Crit );

        // If the user calls pop() more than once after the
        // semaphore is signaled, then the semaphore count will
        // get out of sync.  We fix that when the queue empties.
        if ( __super::empty() )
        {
            do
            {
            } while ( ::WaitForSingleObject( m_hSemaphore, 0 ) !=
                      WAIT_TIMEOUT );

            return false;
        }

        c = __super::front();
        __super::pop_front();

        return true;
    }

    // If overflow, use this to clear the queue.
    void clear()
    {
        //		CComCritSecLock<CComAutoCriticalSection> lock( m_Crit,
        // true );
        MutexLock lock( m_Crit );

        for ( DWORD i = 0; i < __super::size(); i++ )
            ::WaitForSingleObject( m_hSemaphore, 0 );

        __super::clear();

        m_bOverflow = false;
    }

    bool overflow()
    {
        return m_bOverflow;
    }

    HANDLE GetWaitHandle()
    {
        return m_hSemaphore;
    }

protected:
    HANDLE m_hSemaphore;
    //	CComAutoCriticalSection m_Crit;
    typedef std::unique_lock<std::mutex> MutexLock;
    std::mutex                           m_Crit;

    bool m_bOverflow;
};
