#pragma once

/*
 *  Copyright(c) 2015 Jeremiah van Oosten
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
 *  @file LogManager.h
 *  @date December 23, 2015
 *  @author Jeremiah
 *
 *  @brief LogManager class for logging messages to the registered log streams.
 */

#include <sstream>

class LogStream;

enum class LogLevel : uint32_t
{
    Info    = ( 1 << 0 ),
    Warning = ( 1 << 1 ),
    Error   = ( 1 << 2 ),
};

template<>
struct enable_bitmask_operators<LogLevel>
{
    static constexpr bool enable = true;
};

class ENGINE_DLL LogManager : public NonCopyable
{
public:
    static void Init();
    static void RegisterLogStream( std::shared_ptr<LogStream> logStream );
    static void UnregisterLogStream( std::shared_ptr<LogStream> logStream );
    static void Shutdown();

    template<typename... Args>
    static void LogInfo( Args... args );

    template<typename... Args>
    static void LogWarning( Args... args );

    template<typename... Args>
    static void LogError( Args... args );

private:
    static void UnregisterAllStreams();
    static void Log( LogLevel level, const std::wstring& message );
    static void Log( LogLevel level, const std::string& mssage );
};

template<typename... Args>
void LogManager::LogInfo( Args... args )
{
    std::wstring str;

    using ::to_wstring;
    using std::to_wstring;

    int unpack[] { 0, ( str += to_wstring( args ), 0 )... };

    Log( LogLevel::Info, str );
}

template<typename... Args>
void LogManager::LogWarning( Args... args )
{
    std::wstring str;

    using ::to_wstring;
    using std::to_wstring;

    int unpack[] { 0, ( str += to_wstring( args ), 0 )... };

    Log( LogLevel::Warning, str );
}

template<typename... Args>
void LogManager::LogError( Args... args )
{
    std::wstring str;

    using ::to_wstring;
    using std::to_wstring;

    int unpack[] { 0, ( str += to_wstring( args ), 0 )... };

    Log( LogLevel::Error, str );
}

#define LOG_INFO( ... )                                                        \
    LogManager::LogInfo( __FILE__, "(", __LINE__, "): [INFO] ", __FUNCTION__,  \
                         ": ", __VA_ARGS__ )
#define LOG_WARNING( ... )                                                     \
    LogManager::LogWarning( __FILE__, "(", __LINE__, "): [WARNING] ",          \
                            __FUNCTION__, ": ", __VA_ARGS__ )
#define LOG_ERROR( ... )                                                       \
    LogManager::LogError( __FILE__, "(", __LINE__, "): [ERROR] ",              \
                          __FUNCTION__, ": ", __VA_ARGS__ )
