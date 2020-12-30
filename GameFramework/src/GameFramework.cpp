#include "GameFrameworkPCH.h"

#include <GameFramework/GameFramework.h>

#include <GameFramework/Window.h>

static GameFramework* gs_pSingelton       = nullptr;
constexpr wchar_t     WINDOW_CLASS_NAME[] = L"RenderWindowClass";

static LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

// Set the name of an std::thread.
// Useful for debugging.
const DWORD MS_VC_EXCEPTION = 0x406D1388;

// Set the name of a running thread (for debugging)
#pragma pack( push, 8 )
typedef struct tagTHREADNAME_INFO
{
    DWORD  dwType;      // Must be 0x1000.
    LPCSTR szName;      // Pointer to name (in user addr space).
    DWORD  dwThreadID;  // Thread ID (-1=caller thread).
    DWORD  dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack( pop )

inline void SetThreadName( std::thread& thread, const char* threadName )
{
    THREADNAME_INFO info;
    info.dwType     = 0x1000;
    info.szName     = threadName;
    info.dwThreadID = ::GetThreadId( reinterpret_cast<HANDLE>( thread.native_handle() ) );
    info.dwFlags    = 0;

    __try
    {
        ::RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR*)&info );
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
    }
}

constexpr int MAX_CONSOLE_LINES = 500;

using WindowMap       = std::map<HWND, std::weak_ptr<Window>>;
using WindowMapByName = std::map<std::wstring, std::weak_ptr<Window>>;
static WindowMap       gs_WindowMap;
static WindowMapByName gs_WindowMapByName;

static std::mutex gs_WindowHandlesMutex;

// A wrapper struct to allow shared pointers for the window class.
// This is needed because the constructor and destructor for the Window
// class are protected and not accessible by the std::make_shared method.
struct MakeWindow : public Window
{
    MakeWindow( HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight )
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

GameFramework::GameFramework( HINSTANCE hInst )
: m_hInstance( hInst )
, m_bIsRunning( false )
, m_RequestQuit( false )
, m_bTerminateDirectoryChangeThread( false )
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.
    // @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setthreaddpiawarenesscontext
    SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

#if defined( _DEBUG )
    // Create a console window for std::cout
    CreateConsole();
#endif

    // Init spdlog.
    spdlog::init_thread_pool( 8192, 1 );
    auto stdout_sink   = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/log.txt", 1024 * 1024 * 5, 3,
        true );  // Max size: 5MB, Max files: 3, Rotate on open: true
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks { stdout_sink, rotating_sink, msvc_sink };
    m_Logger = std::make_shared<spdlog::async_logger>( "GameFramework", sinks.begin(), sinks.end(),
                                                       spdlog::thread_pool(), spdlog::async_overflow_policy::block );
    spdlog::register_logger( m_Logger );
    spdlog::set_default_logger( m_Logger );

    spdlog::info( "Logging started." );

    // Init gainput.
    m_KeyboardDevice = m_InputManager.CreateDevice<gainput::InputDeviceKeyboard>();
    m_MouseDevice    = m_InputManager.CreateDevice<gainput::InputDeviceMouse>();
    for ( unsigned i = 0; i < gainput::MaxPadCount; ++i )
    { m_GamepadDevice[i] = m_InputManager.CreateDevice<gainput::InputDevicePad>( i ); }

    // This will prevent normalization of mouse coordinates.
    m_InputManager.SetDisplaySize(1, 1);

    // Initializes the COM library for use by the calling thread, sets the thread's concurrency model, and creates a new
    // apartment for the thread if one is required.
    // This must be called at least once for each thread that uses the COM library.
    // @see https://docs.microsoft.com/en-us/windows/win32/api/objbase/nf-objbase-coinitialize
    HRESULT hr = CoInitialize( NULL );
    if ( FAILED( hr ) )
    {
        _com_error err( hr );  // I hope this never happens.
        spdlog::critical( "CoInitialize failed: {}", err.ErrorMessage() );
        throw new std::exception( err.ErrorMessage() );
    }

    WNDCLASSEXW wndClass = { 0 };

    wndClass.cbSize        = sizeof( WNDCLASSEX );
    wndClass.style         = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc   = &WndProc;
    wndClass.hInstance     = m_hInstance;
    wndClass.hCursor       = LoadCursor( nullptr, IDC_ARROW );
    wndClass.hIcon         = LoadIcon( m_hInstance, MAKEINTRESOURCE( APP_ICON ) );
    wndClass.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wndClass.lpszMenuName  = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    wndClass.hIconSm       = LoadIcon( m_hInstance, MAKEINTRESOURCE( APP_ICON ) );

    if ( !RegisterClassExW( &wndClass ) )
    {
        MessageBoxA( NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR );
    }

    // Create a thread to listen for file changes.
    m_DirectoryChangeListenerThread = std::thread( &GameFramework::CheckFileChanges, this );
    SetThreadName( m_DirectoryChangeListenerThread, "Check File Changes" );
}

GameFramework::~GameFramework()
{
    // Close the thread listening for file changes.
    m_bTerminateDirectoryChangeThread = true;
    if ( m_DirectoryChangeListenerThread.joinable() )
    {
        m_DirectoryChangeListenerThread.join();
    }

    gs_WindowMap.clear();
    gs_WindowMapByName.clear();
}

GameFramework& GameFramework::Create( HINSTANCE hInst )
{
    if ( !gs_pSingelton )
    {
        gs_pSingelton = new GameFramework( hInst );
        spdlog::info( "GameFramework class created." );
    }

    return *gs_pSingelton;
}

void GameFramework::Destroy()
{
    if ( gs_pSingelton )
    {
        delete gs_pSingelton;
        gs_pSingelton = nullptr;
        spdlog::info( "GameFramework class destroyed." );
    }
}

GameFramework& GameFramework::Get()
{
    assert( gs_pSingelton != nullptr );
    return *gs_pSingelton;
}

// Create loggers
// @see https://github.com/gabime/spdlog#asynchronous-logger-with-multi-sinks
Logger GameFramework::CreateLogger( const std::string& name )
{
    Logger logger = spdlog::get( name );
    if ( !logger )
    {
        logger = m_Logger->clone( name );
        spdlog::register_logger( logger );
    }

    return logger;
}

gainput::DeviceId GameFramework::GetKeyboardId() const
{
    return m_KeyboardDevice;
}

gainput::DeviceId GameFramework::GetMouseId() const
{
    return m_MouseDevice;
}

gainput::DeviceId GameFramework::GetPadId( unsigned index /*= 0 */ ) const
{
    assert( index >= 0 && index < gainput::MaxPadCount );
    return m_GamepadDevice[index];
}

std::shared_ptr<gainput::InputMap> GameFramework::CreateInputMap( const char* name )
{
    return std::make_shared<gainput::InputMap>( m_InputManager, name );
}

int32_t GameFramework::Run()
{
    assert( !m_bIsRunning );

    m_bIsRunning = true;

    MSG msg = {};
    while ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) && msg.message != WM_QUIT )
    {
        ::TranslateMessage( &msg );
        ::DispatchMessage( &msg );

        m_InputManager.HandleMessage( msg );

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

void GameFramework::SetDisplaySize( int width, int height )
{
    m_InputManager.SetDisplaySize( width, height );
}

void GameFramework::ProcessInput()
{
    m_InputManager.Update();
}

void GameFramework::Stop()
{
    // When called from another thread other than the main thread,
    // the WM_QUIT message goes to that thread and will not be handled
    // in the main thread. To circumvent this, we also set a boolean flag
    // to indicate that the user has requested to quit the application.
    m_RequestQuit = true;
}

std::shared_ptr<Window> GameFramework::CreateWindow( const std::wstring& windowName, int clientWidth, int clientHeight )
{
    int screenWidth  = ::GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = ::GetSystemMetrics( SM_CYSCREEN );

    RECT windowRect = { 0, 0, static_cast<LONG>( clientWidth ), static_cast<LONG>( clientHeight ) };

    ::AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );

    uint32_t width  = windowRect.right - windowRect.left;
    uint32_t height = windowRect.bottom - windowRect.top;

    int windowX = std::max<int>( 0, ( screenWidth - (int)width ) / 2 );
    int windowY = std::max<int>( 0, ( screenHeight - (int)height ) / 2 );

    HWND hWindow = ::CreateWindowExW( NULL, WINDOW_CLASS_NAME, windowName.c_str(), WS_OVERLAPPEDWINDOW, windowX,
                                      windowY, width, height, NULL, NULL, m_hInstance, NULL );

    if ( !hWindow )
    {
        spdlog::error( "Failed to create window." );
        return nullptr;
    }

    auto pWindow = std::make_shared<MakeWindow>( hWindow, windowName, clientWidth, clientHeight );

    gs_WindowMap.insert( WindowMap::value_type( hWindow, pWindow ) );
    gs_WindowMapByName.insert( WindowMapByName::value_type( windowName, pWindow ) );

    return pWindow;
}

std::shared_ptr<Window> GameFramework::GetWindowByName( const std::wstring& windowName ) const
{
    auto iter = gs_WindowMapByName.find( windowName );
    return ( iter != gs_WindowMapByName.end() ) ? iter->second.lock() : nullptr;
}

void GameFramework::RegisterDirectoryChangeListener( const std::wstring& dir, bool recursive )
{
    scoped_lock lock( m_DirectoryChangeMutex );
    m_DirectoryChanges.AddDirectory( dir, recursive, FILE_NOTIFY_CHANGE_LAST_WRITE );
}

// This is the directory change listener thread entry point.
void GameFramework::CheckFileChanges()
{
    while ( !m_bTerminateDirectoryChangeThread )
    {
        scoped_lock lock( m_DirectoryChangeMutex );

        DWORD waitSignal = ::WaitForSingleObject( m_DirectoryChanges.GetWaitHandle(), 0 );
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

                FileChangedEventArgs fileChangedEventArgs( fileAction, fileName );
                OnFileChange( fileChangedEventArgs );
            }

            break;
        default:
            break;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        // std::this_thread::yield();
    }
}

void GameFramework::OnFileChange( FileChangedEventArgs& e )
{
    FileChanged( e );
}

LRESULT GameFramework::OnWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    auto res = WndProcHandler( hWnd, msg, wParam, lParam );
    return res ? *res : 0;
}

void GameFramework::OnExit( EventArgs& e )
{
    // Invoke the Exit event.
    Exit( e );
}

// Convert the message ID into a MouseButton ID
static MouseButton DecodeMouseButton( UINT messageID )
{
    MouseButton mouseButton = MouseButton::None;
    switch ( messageID )
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        mouseButton = MouseButton::Left;
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    {
        mouseButton = MouseButton::Right;
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    {
        mouseButton = MouseButton::Middle;
    }
    break;
    }

    return mouseButton;
}

// Convert the message ID into a ButtonState.
static ButtonState DecodeButtonState( UINT messageID )
{
    ButtonState buttonState = ButtonState::Pressed;

    switch ( messageID )
    {
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        buttonState = ButtonState::Released;
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        buttonState = ButtonState::Pressed;
        break;
    }

    return buttonState;
}

// Convert wParam of the WM_SIZE events to a WindowState.
static WindowState DecodeWindowState( WPARAM wParam )
{
    WindowState windowState = WindowState::Restored;

    switch ( wParam )
    {
    case SIZE_RESTORED:
        windowState = WindowState::Restored;
        break;
    case SIZE_MINIMIZED:
        windowState = WindowState::Minimized;
        break;
    case SIZE_MAXIMIZED:
        windowState = WindowState::Maximized;
        break;
    default:
        break;
    }

    return windowState;
}

static LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    // Allow for external handling of window messages.
    if ( GameFramework::Get().OnWndProc( hwnd, message, wParam, lParam ) )
    {
        return 1;
    }

    std::shared_ptr<Window> pWindow;
    {
        auto iter = gs_WindowMap.find( hwnd );
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
            if ( PeekMessage( &charMsg, hwnd, 0, 0, PM_NOREMOVE ) && charMsg.message == WM_CHAR )
            {
//                GetMessage( &charMsg, hwnd, 0, 0 );
                c = static_cast<unsigned int>( charMsg.wParam );
            }

            bool shift   = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
            bool control = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
            bool alt     = ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0;

            KeyCode      key = (KeyCode)wParam;
            KeyEventArgs keyEventArgs( key, c, KeyState::Pressed, control, shift, alt );
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
                     ToUnicodeEx( (UINT)wParam, scanCode, keyboardState, translatedCharacters, 4, 0, NULL ) > 0 )
            {
                c = translatedCharacters[0];
            }

            KeyEventArgs keyEventArgs( key, c, KeyState::Released, control, shift, alt );
            pWindow->OnKeyReleased( keyEventArgs );
        }
        break;
        // The default window procedure will play a system notification sound
        // when pressing the Alt+Enter keyboard combination if this message is
        // not handled.
        case WM_SYSCHAR:
            break;
        case WM_KILLFOCUS:
        {
            // Window lost keyboard focus.
            EventArgs eventArgs;
            pWindow->OnKeyboardBlur( eventArgs );
        }
        break;
        case WM_SETFOCUS:
        {
            EventArgs eventArgs;
            pWindow->OnKeyboardFocus( eventArgs );
        }
        break;
        case WM_MOUSEMOVE:
        {
            bool lButton = ( wParam & MK_LBUTTON ) != 0;
            bool rButton = ( wParam & MK_RBUTTON ) != 0;
            bool mButton = ( wParam & MK_MBUTTON ) != 0;
            bool shift   = ( wParam & MK_SHIFT ) != 0;
            bool control = ( wParam & MK_CONTROL ) != 0;

            int x = ( (int)(short)LOWORD( lParam ) );
            int y = ( (int)(short)HIWORD( lParam ) );

            MouseMotionEventArgs mouseMotionEventArgs( lButton, mButton, rButton, control, shift, x, y );
            pWindow->OnMouseMoved( mouseMotionEventArgs );
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            bool lButton = ( wParam & MK_LBUTTON ) != 0;
            bool rButton = ( wParam & MK_RBUTTON ) != 0;
            bool mButton = ( wParam & MK_MBUTTON ) != 0;
            bool shift   = ( wParam & MK_SHIFT ) != 0;
            bool control = ( wParam & MK_CONTROL ) != 0;

            int x = ( (int)(short)LOWORD( lParam ) );
            int y = ( (int)(short)HIWORD( lParam ) );

            // Capture mouse movement until the button is released.
            SetCapture( hwnd );

            MouseButtonEventArgs mouseButtonEventArgs( DecodeMouseButton( message ), ButtonState::Pressed, lButton,
                                                       mButton, rButton, control, shift, x, y );
            pWindow->OnMouseButtonPressed( mouseButtonEventArgs );
        }
        break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            bool lButton = ( wParam & MK_LBUTTON ) != 0;
            bool rButton = ( wParam & MK_RBUTTON ) != 0;
            bool mButton = ( wParam & MK_MBUTTON ) != 0;
            bool shift   = ( wParam & MK_SHIFT ) != 0;
            bool control = ( wParam & MK_CONTROL ) != 0;

            int x = ( (int)(short)LOWORD( lParam ) );
            int y = ( (int)(short)HIWORD( lParam ) );

            // Stop capturing the mouse.
            ReleaseCapture();

            MouseButtonEventArgs mouseButtonEventArgs( DecodeMouseButton( message ), ButtonState::Released, lButton,
                                                       mButton, rButton, control, shift, x, y );
            pWindow->OnMouseButtonReleased( mouseButtonEventArgs );
        }
        break;
        case WM_MOUSEWHEEL:
        {
            // The distance the mouse wheel is rotated.
            // A positive value indicates the wheel was rotated forwards (away
            //  the user). A negative value indicates the wheel was rotated
            //  backwards (toward the user).
            float zDelta    = ( (int)(short)HIWORD( wParam ) ) / (float)WHEEL_DELTA;
            short keyStates = (short)LOWORD( wParam );

            bool lButton = ( keyStates & MK_LBUTTON ) != 0;
            bool rButton = ( keyStates & MK_RBUTTON ) != 0;
            bool mButton = ( keyStates & MK_MBUTTON ) != 0;
            bool shift   = ( keyStates & MK_SHIFT ) != 0;
            bool control = ( keyStates & MK_CONTROL ) != 0;

            int x = ( (int)(short)LOWORD( lParam ) );
            int y = ( (int)(short)HIWORD( lParam ) );

            // Convert the screen coordinates to client coordinates.
            POINT screenToClientPoint;
            screenToClientPoint.x = x;
            screenToClientPoint.y = y;
            ::ScreenToClient( hwnd, &screenToClientPoint );

            MouseWheelEventArgs mouseWheelEventArgs( zDelta, lButton, mButton, rButton, control, shift,
                                                     (int)screenToClientPoint.x, (int)screenToClientPoint.y );
            pWindow->OnMouseWheel( mouseWheelEventArgs );
        }
        break;
        // NOTE: Not really sure if these next set of messages are working
        // correctly. Not really sure HOW to get them to work correctly.
        // TODO: Try to fix these if I need them ;)
        case WM_CAPTURECHANGED:
        {
            EventArgs mouseBlurEventArgs;
            pWindow->OnMouseBlur( mouseBlurEventArgs );
        }
        break;
        case WM_MOUSEACTIVATE:
        {
            EventArgs mouseFocusEventArgs;
            pWindow->OnMouseFocus( mouseFocusEventArgs );
        }
        break;
        case WM_MOUSELEAVE:
        {
            EventArgs mouseLeaveEventArgs;
            pWindow->OnMouseLeave( mouseLeaveEventArgs );
        }
        break;
        case WM_SIZE:
        {
            WindowState windowState = DecodeWindowState( wParam );

            int width  = ( (int)(short)LOWORD( lParam ) );
            int height = ( (int)(short)HIWORD( lParam ) );

            ResizeEventArgs resizeEventArgs( width, height, windowState );
            pWindow->OnResize( resizeEventArgs );
        }
        break;
        case WM_CLOSE:
        {
            WindowCloseEventArgs windowCloseEventArgs;
            pWindow->OnClose( windowCloseEventArgs );

            // Check to see if the user canceled the close event.
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