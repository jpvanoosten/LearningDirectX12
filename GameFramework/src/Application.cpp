#include <GameFrameworkPCH.h>

#include <Application.h>

#include <Window.h>

static Application* gs_pSingelton       = nullptr;
constexpr wchar_t   WINDOW_CLASS_NAME[] = L"RenderWindowClass";

static LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam,
                                 LPARAM lParam );

constexpr int MAX_CONSOLE_LINES = 500;

using WindowMap = std::map<HWND, std::weak_ptr<Window>>;
static WindowMap gs_WindowMap;
std::mutex       gs_WindowHandlesMutex;

// A wrapper struct to allow shared pointers for the window class.
// This is needed because the constructor and destructor for the Window
// class are protected and not accessible by the std::make_shared method.
struct MakeWindow : public Window
{
    MakeWindow( HWND hWnd, const std::wstring& windowName, int clientWidth,
                int clientHeight )
    : Window( hWnd, windowName, clientWidth, clientHeight )
    {}
};

/**
 * Create a console window (consoles are not automatically created for Windows
 * subsystems)
 */
static void CreateConsole()
{
    // Allocate a console.
    if ( AllocConsole() )
    {
        HANDLE lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );

        // Increase screen buffer to allow more lines of text than the default.
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo( lStdHandle, &consoleInfo );
        consoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
        SetConsoleScreenBufferSize( lStdHandle, consoleInfo.dwSize );
        SetConsoleCursorPosition( lStdHandle, { 0, 0 } );

        // Redirect unbuffered STDOUT to the console.
        int   hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
        FILE* fp         = _fdopen( hConHandle, "w" );
        freopen_s( &fp, "CONOUT$", "w", stdout );
        setvbuf( stdout, nullptr, _IONBF, 0 );

        // Redirect unbuffered STDIN to the console.
        lStdHandle = GetStdHandle( STD_INPUT_HANDLE );
        hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
        fp         = _fdopen( hConHandle, "r" );
        freopen_s( &fp, "CONIN$", "r", stdin );
        setvbuf( stdin, nullptr, _IONBF, 0 );

        // Redirect unbuffered STDERR to the console.
        lStdHandle = GetStdHandle( STD_ERROR_HANDLE );
        hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
        fp         = _fdopen( hConHandle, "w" );
        freopen_s( &fp, "CONOUT$", "w", stderr );
        setvbuf( stderr, nullptr, _IONBF, 0 );

        // Clear the error state for each of the C++ standard stream objects. We
        // need to do this, as attempts to access the standard streams before
        // they refer to a valid target will cause the iostream objects to enter
        // an error state. In versions of Visual Studio after 2005, this seems
        // to always occur during startup regardless of whether anything has
        // been read from or written to the console or not.
        std::wcout.clear();
        std::cout.clear();
        std::wcerr.clear();
        std::cerr.clear();
        std::wcin.clear();
        std::cin.clear();
    }
}

Application::Application( HINSTANCE hInst )
: m_hInstance( hInst )
, m_bIsRunning( false )
, m_RequestQuit( false )
, m_bTerminateDirectoryChangeThread( false )
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

    CreateConsole();

    // Create loggers
    // @see
    // https://github.com/gabime/spdlog#asynchronous-logger-with-multi-sinks
    spdlog::init_thread_pool( 8192, 1 );
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/log.txt", 1024 * 1024 * 5, 3,
        true );  // Max size: 5MB, Max files: 3, Rotate on open: true
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks { stdout_sink, rotating_sink,
                                          msvc_sink };
    auto logger = std::make_shared<spdlog::async_logger>(
        "Application", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block );
    spdlog::register_logger( logger );
    spdlog::set_default_logger( logger );

    spdlog::info( L"Wide charcter format string: {}",
                  L"Wide character string" );

    WNDCLASSEXW wndClass = { 0 };

    wndClass.cbSize      = sizeof( WNDCLASSEX );
    wndClass.style       = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance   = m_hInstance;
    wndClass.hCursor     = LoadCursor( nullptr, IDC_ARROW );
    wndClass.hIcon       = LoadIcon( m_hInstance, MAKEINTRESOURCE( APP_ICON ) );
    wndClass.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wndClass.lpszMenuName  = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    wndClass.hIconSm = LoadIcon( m_hInstance, MAKEINTRESOURCE( APP_ICON ) );

    if ( !RegisterClassExW( &wndClass ) )
    {
        MessageBoxA( NULL, "Unable to register the window class.", "Error",
                     MB_OK | MB_ICONERROR );
    }

    // Create a thread to listen for file changes.
    m_DirectoryChangeListenerThread =
        std::thread( &Application::CheckFileChanges, this );
}

Application::~Application()
{
    // Close the thread listening for file changes.
    m_bTerminateDirectoryChangeThread = true;
    if ( m_DirectoryChangeListenerThread.joinable() )
    {
        m_DirectoryChangeListenerThread.join();
    }

    gs_WindowMap.clear();
}

Application& Application::Create( HINSTANCE hInst )
{
    if ( !gs_pSingelton )
    {
        gs_pSingelton = new Application( hInst );
        spdlog::info( "Application class created." );
    }

    return *gs_pSingelton;
}

void Application::Destroy()
{
    if ( gs_pSingelton )
    {
        delete gs_pSingelton;
        gs_pSingelton = nullptr;
        spdlog::info( "Application class destroyed." );
    }
}

Application& Application::Get()
{
    assert( gs_pSingelton != nullptr );
    return *gs_pSingelton;
}

int32_t Application::Run()
{
    assert( !m_bIsRunning );

    m_bIsRunning = true;

    MSG msg      = {};
    while ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) && msg.message != WM_QUIT )
    {
        ::TranslateMessage( &msg );
        ::DispatchMessage( &msg );

        // Check to see of the application wants to quit.
        if ( m_RequestQuit )
        {
            ::PostQuitMessage( 0 );
            m_RequestQuit = false;
        }
    }

    m_bIsRunning = false;

    return static_cast<int32_t>( msg.wParam );
}

void Application::Stop()
{
    // When called from another thread other than the main thread,
    // the WM_QUIT message goes to that thread and will not be handled
    // in the main thread. To circumvent this, we also set a boolean flag
    // to indicate that the user has requested to quit the application.
    m_RequestQuit = true;
}

std::shared_ptr<Window>
    Application::CreateWindow( const std::wstring& windowName, int clientWidth,
                               int clientHeight )
{
    int screenWidth  = ::GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = ::GetSystemMetrics( SM_CYSCREEN );

    RECT windowRect = { 0, 0, static_cast<LONG>( clientWidth ),
                        static_cast<LONG>( clientHeight ) };

    ::AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );

    uint32_t width  = windowRect.right - windowRect.left;
    uint32_t height = windowRect.bottom - windowRect.top;

    int windowX = std::max<int>( 0, ( screenWidth - (int)width ) / 2 );
    int windowY = std::max<int>( 0, ( screenHeight - (int)height ) / 2 );

    HWND hWindow = ::CreateWindowExW(
        NULL, WINDOW_CLASS_NAME, windowName.c_str(), WS_OVERLAPPEDWINDOW,
        windowX, windowY, width, height, NULL, NULL, m_hInstance, NULL );

    if ( !hWindow )
    {
        spdlog::error( "Failed to create window." );
        return nullptr;
    }

    auto pWindow = std::make_shared<MakeWindow>( hWindow, windowName,
                                                 clientWidth, clientHeight );

    gs_WindowMap.insert( WindowMap::value_type( hWindow, pWindow ) );

    return pWindow;
}

void Application::RegisterDirectoryChangeListener( const std::wstring& dir,
                                                   bool recursive )
{
    scoped_lock lock( m_DirectoryChangeMutex );
    m_DirectoryChanges.AddDirectory( dir, recursive,
                                     FILE_NOTIFY_CHANGE_LAST_WRITE );
}

// This is the directory change listener thread entry point.
void Application::CheckFileChanges()
{
    while ( !m_bTerminateDirectoryChangeThread )
    {
        scoped_lock lock( m_DirectoryChangeMutex );

        DWORD waitSignal =
            ::WaitForSingleObject( m_DirectoryChanges.GetWaitHandle(), 0 );
        switch ( waitSignal )
        {
        case WAIT_OBJECT_0:
            // A file has been modified
            if ( m_DirectoryChanges.CheckOverflow() )
            {
                // This could happen if a lot of modifications occur at once.
                spdlog::warn( "Directory change overflow occurred." );
            }
            else
            {
                DWORD        action;
                std::wstring fileName;
                m_DirectoryChanges.Pop( action, fileName );

                FileAction fileAction = FileAction::Unknown;
                switch ( action )
                {
                case FILE_ACTION_ADDED:
                    fileAction = FileAction::Added;
                    break;
                case FILE_ACTION_REMOVED:
                    fileAction = FileAction::Removed;
                    break;
                case FILE_ACTION_MODIFIED:
                    fileAction = FileAction::Modified;
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                    fileAction = FileAction::RenameOld;
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    fileAction = FileAction::RenameNew;
                    break;
                default:
                    break;
                }

                FileChangedEventArgs fileChangedEventArgs( fileAction,
                                                          fileName );
                OnFileChange( fileChangedEventArgs );
            }

            break;
        default:
            break;
        }

        std::this_thread::yield();
    }
}

void Application::OnFileChange( FileChangedEventArgs& e )
{
    FileChanged( e );
}

void Application::OnExit( EventArgs& e )
{
    // Invoke the Exit event.
    Exit( e );
}

static LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam,
                                 LPARAM lParam )
{
    std::shared_ptr<Window> pWindow;
    {
        WindowMap::const_iterator iter = gs_WindowMap.find( hwnd );
        if ( iter != gs_WindowMap.end() )
        {
            pWindow = iter->second.lock();
        }
    }

    if ( pWindow )
    {

        switch ( message )
        {
        case WM_DPICHANGED:
        {
            float             dpiScaling = HIWORD( wParam ) / 96.0f;
            DPIScaleEventArgs dpiScaleEventArgs( dpiScaling );
            pWindow->OnDPIScaleChanged( dpiScaleEventArgs );
        }
        break;
        case WM_PAINT:
        {
            // Delta and total time will be filled in by the Window.
            UpdateEventArgs updateEventArgs( 0.0, 0.0 );
            pWindow->OnUpdate( updateEventArgs );
        }
        break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            MSG charMsg;

            // Get the Unicode character (UTF-16)
            unsigned int c = 0;
            // For printable characters, the next message will be WM_CHAR.
            // This message contains the character code we need to send the
            // KeyPressed event. Inspired by the SDL 1.2 implementation.
            if ( PeekMessage( &charMsg, hwnd, 0, 0, PM_NOREMOVE ) &&
                 charMsg.message == WM_CHAR )
            {
                GetMessage( &charMsg, hwnd, 0, 0 );
                c = static_cast<unsigned int>( charMsg.wParam );
            }

            bool shift   = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
            bool control = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
            bool alt     = ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0;

            KeyCode      key      = (KeyCode)wParam;
            unsigned int scanCode = ( lParam & 0x00FF0000 ) >> 16;
            KeyEventArgs keyEventArgs( key, c, KeyState::Pressed, control,
                                       shift, alt );
            pWindow->OnKeyPressed( keyEventArgs );
        }
        break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            bool shift   = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
            bool control = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
            bool alt     = ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0;

            KeyCode      key      = (KeyCode)wParam;
            unsigned int c        = 0;
            unsigned int scanCode = ( lParam & 0x00FF0000 ) >> 16;

            // Determine which key was released by converting the key code and
            // the scan code to a printable character (if possible). Inspired by
            // the SDL 1.2 implementation.
            unsigned char keyboardState[256];
            GetKeyboardState( keyboardState );
            wchar_t translatedCharacters[4];
            if ( int result =
                     ToUnicodeEx( (UINT)wParam, scanCode, keyboardState,
                                  translatedCharacters, 4, 0, NULL ) > 0 )
            {
                c = translatedCharacters[0];
            }

            KeyEventArgs keyEventArgs( key, c, KeyState::Released, control,
                                       shift, alt );
            pWindow->OnKeyReleased( keyEventArgs );
        }
        break;
        case WM_SIZE:
        {
            int width  = ( (int)(short)LOWORD( lParam ) );
            int height = ( (int)(short)HIWORD( lParam ) );

            ResizeEventArgs resizeEventArgs( width, height );
            pWindow->OnResize( resizeEventArgs );
        }
        break;
        case WM_CLOSE:
        {
            // This message cannot be queued on the message queue because we
            // need the response from the event arguments to determine if the
            // window should really be closed or not.
            WindowCloseEventArgs windowCloseEventArgs;
            pWindow->OnClose( windowCloseEventArgs );

            if ( windowCloseEventArgs.ConfirmClose )
            {
                // DestroyWindow( hwnd );
                // Just hide the window.
                // Windows will be destroyed when the application quits.
                pWindow->Hide();
            }
        }
        break;
        case WM_DESTROY:
        {
            std::lock_guard<std::mutex> lock( gs_WindowHandlesMutex );
            WindowMap::iterator         iter = gs_WindowMap.find( hwnd );
            if ( iter != gs_WindowMap.end() )
            {
                gs_WindowMap.erase( iter );
            }
        }
        break;
        default:
            return ::DefWindowProcW( hwnd, message, wParam, lParam );
        }
    }
    else
    {
        switch ( message )
        {
        case WM_CREATE:
            break;
        default:
            return ::DefWindowProcW( hwnd, message, wParam, lParam );
        }
    }

    return 0;
}