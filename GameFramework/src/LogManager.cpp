#include <EnginePCH.h>

#include <LogManager.h>
#include <LogStream.h>
#include <Common.h>
#include <EngineDefines.h>
#include <ThreadSafeQueue.h>

using namespace Core;

using LogStreamList = std::vector< std::shared_ptr<LogStream> >;

struct LogMessage
{
    LogLevel m_LogLevel;
    std::wstring m_Message;
};

static LogStreamList gs_LogStreams;
static ThreadSafeQueue<LogMessage> gs_MessageQueue;
static std::mutex gs_LogStreamsMutex;
static std::atomic_bool g_ProcessMessages = true;
static std::thread g_MessageThread;

void ProcessMessagesFunc()
{
    while ( g_ProcessMessages )
    {
        LogMessage msg;
        while ( gs_MessageQueue.TryPop( msg ) )
        {
            scoped_lock lock( gs_LogStreamsMutex );
            for ( auto log : gs_LogStreams )
            {
                log->Write( msg.m_LogLevel, msg.m_Message );
            }
        }

        std::this_thread::yield();
    }
}

void LogManager::Init()
{
    g_MessageThread = std::thread( &ProcessMessagesFunc );
}

void LogManager::RegisterLogStream( std::shared_ptr<LogStream> logStream )
{
    scoped_lock lock( gs_LogStreamsMutex );
    gs_LogStreams.push_back( logStream );
}

void LogManager::UnregisterLogStream( std::shared_ptr<LogStream> logStream )
{
    scoped_lock lock( gs_LogStreamsMutex );
    LogStreamList::const_iterator iter = std::find( gs_LogStreams.begin(), gs_LogStreams.end(), logStream );
    if ( iter != gs_LogStreams.end() )
    {
        gs_LogStreams.erase( iter );
    }
}

void LogManager::Shutdown()
{
    g_ProcessMessages = false;
    if ( g_MessageThread.joinable() )
    {
        g_MessageThread.join();
    }

    UnregisterAllStreams();
}

void LogManager::UnregisterAllStreams()
{
    scoped_lock lock( gs_LogStreamsMutex );
    gs_LogStreams.clear();
}

void LogManager::Log( LogLevel level, const std::wstring& message )
{
    std::wstring msg = message + L"\n";
    gs_MessageQueue.Push( { level, msg } );
}

void LogManager::Log( LogLevel level, const std::string& message )
{
    Log( level, ConvertString( message ) );
}
