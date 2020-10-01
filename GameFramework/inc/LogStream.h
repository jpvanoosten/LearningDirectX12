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
 *  @file LogStream.h
 *  @date December 23, 2015
 *  @author Jeremiah
 *
 *  @brief A log stream class that can be registered with the LogManager to enable logging to multiple sources.
 */

#include "Object.h"
#include "LogManager.h"

#include <fstream>
#include <mutex>

namespace Core
{
    /**
     * Base class for log stream implementations.
     */
    class ENGINE_DLL LogStream : public Object
    {
    public:
        /**
         * Write a message to the log stream.
         */
        virtual void Write( LogLevel level, const std::wstring& message ) = 0;
    };

    /**
     * Log stream that outputs messages to a file.
     */
    class ENGINE_DLL LogStreamFile : public LogStream
    {
    public:
        LogStreamFile( const std::wstring& fileName );
        virtual ~LogStreamFile();

        virtual void Write( LogLevel level, const std::wstring& message ) override;

    private:
        std::mutex m_FileMutex;   // Protect access to file.
        std::wofstream m_ofs;
    };

    /**
     * Log stream that outputs messages to standard output (console)
     */
    class ENGINE_DLL LogStreamConsole : public LogStream
    {
    public:
        LogStreamConsole();
        virtual ~LogStreamConsole() = default;

        virtual void Write( LogLevel level, const std::wstring& message ) override;
    };

    /**
    * Log stream that outputs messages to OutputDebugString available when running from the Visual Studio ID
    */
    class ENGINE_DLL LogStreamVS : public LogStream
    {
    public:
        LogStreamVS() = default;
        virtual ~LogStreamVS() = default;

        virtual void Write( LogLevel level, const std::wstring& message ) override;
    };

    /**
    * Log stream that outputs messages to MessageBox.
    */
    class ENGINE_DLL LogStreamMessageBox : public LogStream
    {
    public:
        /**
         * Displays a message box when writing to one of the specified log levels.
         */
        LogStreamMessageBox( LogLevel levels = LogLevel::Error );
        virtual ~LogStreamMessageBox() = default;

        virtual void Write( LogLevel level, const std::wstring& message ) override;

    private:
        LogLevel m_LogLevels;
    };
}
