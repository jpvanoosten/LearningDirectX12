//
// The MIT License
//
// Copyright (c) 2010 James E Beveridge
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// This sample code is for my blog entry titled, "Understanding ReadDirectoryChangesW"
// http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html
// See ReadMe.txt for overview information.

#include "GameFrameworkPCH.h"

#include "ReadDirectoryChangesPrivate.h"

#include <GameFramework/ReadDirectoryChanges.h>

using namespace ReadDirectoryChangesPrivate;

//
// Usage: SetThreadName ((DWORD)-1, "MainThread");
// Source: https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
#include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack( push, 8 )
typedef struct tagTHREADNAME_INFO
{
    DWORD  dwType;      // Must be 0x1000.
    LPCSTR szName;      // Pointer to name (in user addr space).
    DWORD  dwThreadID;  // Thread ID (-1=caller thread).
    DWORD  dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack( pop )
void SetThreadName( DWORD dwThreadID, const char* threadName )
{
    THREADNAME_INFO info;
    info.dwType     = 0x1000;
    info.szName     = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags    = 0;
#pragma warning( push )
#pragma warning( disable : 6320 6322 )
    __try
    {
        RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR*)&info );
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
    }
#pragma warning( pop )
}

///////////////////////////////////////////////////////////////////////////
// CReadDirectoryChanges

CReadDirectoryChanges::CReadDirectoryChanges( int nMaxCount )
: m_Notifications( nMaxCount )
{
    m_hThread    = NULL;
    m_dwThreadId = 0;
    m_pServer    = new CReadChangesServer( this );
}

CReadDirectoryChanges::~CReadDirectoryChanges()
{
    Terminate();
    delete m_pServer;
}

void CReadDirectoryChanges::Init()
{
    //
    // Kick off the worker thread, which will be
    // managed by CReadChangesServer.
    //
    m_hThread = (HANDLE)_beginthreadex( NULL, 0, CReadChangesServer::ThreadStartProc, m_pServer, 0, &m_dwThreadId );
}

void CReadDirectoryChanges::Terminate()
{
    if ( m_hThread )
    {
        ::QueueUserAPC( CReadChangesServer::TerminateProc, m_hThread, (ULONG_PTR)m_pServer );
        ::WaitForSingleObjectEx( m_hThread, 10000, true );
        ::CloseHandle( m_hThread );

        m_hThread    = NULL;
        m_dwThreadId = 0;
    }
}

void CReadDirectoryChanges::AddDirectory( const std::wstring& szDirectory, BOOL bWatchSubtree, DWORD dwNotifyFilter,
                                          DWORD dwBufferSize )
{
    if ( !m_hThread )
        Init();

    if ( m_hThread )
    {
        CReadChangesRequest* pRequest =
            new CReadChangesRequest( m_pServer, szDirectory, bWatchSubtree, dwNotifyFilter, dwBufferSize );
        ::QueueUserAPC( CReadChangesServer::AddDirectoryProc, m_hThread, (ULONG_PTR)pRequest );
    }
}

void CReadDirectoryChanges::Push( DWORD dwAction, const std::wstring& wstrFilename )
{
    m_Notifications.push( TDirectoryChangeNotification( dwAction, wstrFilename ) );
}

bool CReadDirectoryChanges::Pop( DWORD& dwAction, std::wstring& wstrFilename )
{
    TDirectoryChangeNotification pair;
    if ( !m_Notifications.pop( pair ) )
        return false;

    dwAction     = pair.first;
    wstrFilename = pair.second;

    return true;
}

bool CReadDirectoryChanges::CheckOverflow()
{
    bool b = m_Notifications.overflow();
    if ( b )
        m_Notifications.clear();
    return b;
}
