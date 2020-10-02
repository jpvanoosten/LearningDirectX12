#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
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
 *  @file Application.h
 *  @date September 29, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief The application class is used to create windows for our application.
 */

#include "Events.h"
#include "ReadDirectoryChanges.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// I want to use a function with this name but it conflicts with the Windows
// macro defined in the Windows header files.
#ifdef CreateWindow
    #undef CreateWindow
#endif

#include <cstdint> // for uint32_t
#include <memory>  // for std::shared_ptr
#include <mutex>   // for std::mutex
#include <string>  // for std::wstring
#include <thread>  // for std::thread

class Window;

class Application
{
public:
    /**
     * Create the singleton application instance.
     *
     * @parm hInst The application instance.
     * @returns A reference to the created instance.
     */
    static Application& Create( HINSTANCE hInst );

    /**
     * Destroy the Application instance.
     */
    static void Destroy();

    /**
     * Get a reference to the application instance.
     * @returns A reference to the Application instance.
     */
    static Application& Get();

    /**
     * Start the main application run loop.
     * 
     * @returns The error code (if an error occurred).
     */
    int32_t Run();

    /**
     * Stop the application.
     */
    void Stop();

    /**
     * To support hot-loading support, you can register a directory path
     * for listening for file change notifications. File change notifications
     * are set through the Application::FileChange event.
     *
     * @param dir The directory to listen for file changes.
     * @param recursive Whether to listen for file changes in sub-folders.
     */
    void RegisterDirectoryChangeListener( const std::wstring& dir,
                                          bool recursive = true );

    /**
     * Create a render window.
     *
     * @param windowName The name of the window instance. This will also be the
     * name that appears in the title of the Window.
     * @param clientWidth The width (in pixels) of the window's client area.
     * @param clientHeight The height (in pixels) of the window's client area.
     * @returns The created window instance.
     */
    std::shared_ptr<Window> CreateWindow( const std::wstring& windowName,
                                          int clientWidth, int clientHeight );

    /**
     * Destroy a window by it's name.
     *
     * @param windowName The name that was used to create the window.
     */
    void DestroyWindow( const std::wstring& windowName );

    /**
     * Destroy a window instance.
     *
     * @param pWindow A pointer to the window instance to destroy.
     */
    void DestroyWindow( std::shared_ptr<Window> pWindow );

    /**
     * Get a window by name.
     *
     * @param windowName The name that was used to create the window.
     */
    std::shared_ptr<Window> GetWindowByName( const std::wstring& windowName );

    /**
     * Invoked when a file is modified on disk.
     */
    FileChangeEvent FileChanged;

    /**
     * Application is exiting.
     */
    Event Exit;

protected:
    Application( HINSTANCE hInst );
    virtual ~Application();

    // A file modification was detected.
    virtual void OnFileChange( FileChangeEventArgs& e );

    // Application is going to close
    virtual void OnExit( EventArgs& e );

private:
    // Private and deleted. Please don't try to create copies
    // of this singleton!
    Application( const Application& ) = delete;
    Application( Application&& )      = delete;
    Application& operator=( Application& ) = delete;
    Application& operator=( Application&& ) = delete;

    // Directory change listener thread entry point function.
    void CheckFileChanges();

    // Handle to application instance.
    HINSTANCE m_hInstance;

    // Set to true while the application is running.
    std::atomic_bool m_bIsRunning;
    // Should the application quit?
    std::atomic_bool m_RequestQuit;

    // Directory change listener.
    CReadDirectoryChanges m_DirectoryChanges;
    // Thread to run directory change listener.
    std::thread m_DirectoryChangeListenerThread;
    std::mutex  m_DirectoryChangeMutex;
    // Flag to terminate directory change thread.
    std::atomic_bool m_bTerminateDirectoryChangeThread;
};