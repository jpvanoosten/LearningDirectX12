#include <GameFramework.h>
#include <Window.h>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <shellapi.h>  // for CommandLineToArgvW

void OnFileChanged( FileChangedEventArgs& e );
void OnUpdate( UpdateEventArgs& e );
void OnKeyPressed( KeyEventArgs& e );
void OnKeyReleased( KeyEventArgs& e );
void OnKeyboardFocus( EventArgs& e );
void OnKeyboardBlur( EventArgs& e );

void OnMouseMoved( MouseMotionEventArgs& e );
void OnMouseEnter( MouseMotionEventArgs& e );
void OnMouseButtonPressed( MouseButtonEventArgs& e );
void OnMouseButtonReleased( MouseButtonEventArgs& e );
void OnMouseWheel( MouseWheelEventArgs& e );
void OnMouseLeave( EventArgs& e );
void OnMouseFocus( EventArgs& e );
void OnMouseBlur( EventArgs& e );

void OnWindowResized( ResizeEventArgs& e );
void OnWindowMinimized( ResizeEventArgs& e );
void OnWindowMaximized( ResizeEventArgs& e );
void OnWindowRestored( ResizeEventArgs& e );
void OnWindowClose( WindowCloseEventArgs& e );

std::shared_ptr<Window> pGameWindow = nullptr;
Logger                  logger;

int CALLBACK wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       PWSTR lpCmdLine, int nCmdShow )
{
    int retCode = 0;

    WCHAR   path[MAX_PATH];
    int     argc = 0;
    LPWSTR* argv = ::CommandLineToArgvW( lpCmdLine, &argc );
    if ( argv )
    {
        for ( int i = 0; i < argc; ++i )
        {
            // -wd Specify the Working Directory.
            if ( wcscmp( argv[i], L"-wd" ) == 0 )
            {
                wcscpy_s( path, argv[++i] );
                SetCurrentDirectoryW( path );
            }
        }
        LocalFree( argv );
    }

    auto& gf = GameFramework::Create( hInstance );
    {
        // Create a logger for logging messages.
        logger = gf.CreateLogger( "ClearScreen" );

        // Listen for file changes in the "Assets" folder.
        gf.RegisterDirectoryChangeListener( L"Assets" );
        gf.FileChanged += &OnFileChanged;

        // Create a window:
        pGameWindow = gf.CreateWindow( L"Clear Screen", 1920, 1080 );

        // Register events.
        pGameWindow->KeyPressed += &OnKeyPressed;
        pGameWindow->KeyReleased += &OnKeyReleased;
        pGameWindow->KeyboardFocus += &OnKeyboardFocus;
        pGameWindow->KeyboardBlur += &OnKeyboardBlur;
        pGameWindow->MouseMoved += &OnMouseMoved;
        pGameWindow->MouseButtonPressed += &OnMouseButtonPressed;
        pGameWindow->MouseButtonReleased += &OnMouseButtonReleased;
        pGameWindow->MouseWheel += &OnMouseWheel;
        pGameWindow->MouseEnter += &OnMouseEnter;
        pGameWindow->MouseLeave += &OnMouseLeave;
        pGameWindow->Close += &OnWindowClose;
        pGameWindow->Resize += &OnWindowResized;
        pGameWindow->Minimized += &OnWindowMinimized;
        pGameWindow->Maximized += &OnWindowMaximized;
        pGameWindow->Restored += &OnWindowRestored;
        pGameWindow->Update += &OnUpdate;

        pGameWindow->Show();

        retCode = GameFramework::Get().Run();

        pGameWindow.reset();
    }
    GameFramework::Destroy();

    return retCode;
}

void OnUpdate( UpdateEventArgs& e )
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.DeltaTime;
    frameCount++;

    // Process controller input.
    GameFramework::Get().ProcessInput();

    if ( totalTime > 1.0 )
    {
        auto fps   = frameCount / totalTime;
        frameCount = 0;
        totalTime -= 1.0;

        logger->info( "FPS: {:.7}", fps );
    }


}

void OnKeyPressed( KeyEventArgs& e )
{
    spdlog::info( L"KeyPressed: {}", (wchar_t)e.Char );

    switch ( e.Key )
    {
    case KeyCode::Escape:
        // Stop the application if the Escape key is pressed.
        GameFramework::Get().Stop();
        break;
    case KeyCode::Enter:
        if ( e.Alt )
        {
        case KeyCode::F11:
            pGameWindow->ToggleFullscreen();
            break;
        }
    }
}

void OnKeyReleased( KeyEventArgs& e )
{
    logger->info( L"KeyReleased: {}", (wchar_t)e.Char );
}

void OnKeyboardFocus( EventArgs& e )
{
    logger->info( "KeyboardFocus" );
}

void OnKeyboardBlur( EventArgs& e )
{
    logger->info( "KeyboardBlur" );
}

void OnMouseMoved( MouseMotionEventArgs& e )
{
    logger->info( "MouseMoved: {}, {} ({}, {})", e.X, e.Y, e.RelX, e.RelY );
}

// Convert MouseButton to string (for logging purposes)
template<typename OStream>
OStream& operator<<( OStream& os, MouseButton mb )
{
    switch ( mb )
    {
    case MouseButton::Left:
        os << "Left";
        break;
    case MouseButton::Right:
        os << "Right";
        break;
    case MouseButton::Middle:
        os << "Middle";
        break;
    default:
        break;
    }

    return os;
}

void OnMouseButtonPressed( MouseButtonEventArgs& e )
{
    logger->info( "MouseButtonPressed: {}", e.Button );
}

void OnMouseButtonReleased( MouseButtonEventArgs& e )
{
    logger->info( "MouseButtonReleased: {}", e.Button );
}

void OnMouseWheel( MouseWheelEventArgs& e )
{
    logger->info( "MouseWheel: {}", e.WheelDelta );
}

void OnMouseLeave( EventArgs& e )
{
    logger->info( "MouseLeave" );
}

void OnMouseEnter( MouseMotionEventArgs& e )
{
    logger->info( "MouseEnter: {}, {}", e.X, e.Y );
}

void OnMouseFocus( EventArgs& e )
{
    logger->info( "MouseFocus" );
}

void OnMouseBlur( EventArgs& e )
{
    logger->info( "MouseBlur" );
}

void OnWindowResized( ResizeEventArgs& e )
{
    logger->info( "Window Resize: {}, {}", e.Width, e.Height );
    GameFramework::Get().SetDisplaySize( e.Width, e.Height );
}

void OnWindowMinimized( ResizeEventArgs& e )
{
    logger->info( "Window Minimized" );
}

void OnWindowMaximized( ResizeEventArgs& e )
{
    logger->info( "Window Maximized" );
}

void OnWindowRestored( ResizeEventArgs& e )
{
    logger->info( "Window Restored" );
}

void OnWindowClose( WindowCloseEventArgs& e )
{
    // Stop the application if the window is closed.
    GameFramework::Get().Stop();
}

// Convert FileAction to string (for logging purposes)
template<typename OStream>
OStream& operator<<( OStream& os, FileAction fa )
{
    switch ( fa )
    {
    case FileAction::Unknown:
        os << "Unknown";
        break;
    case FileAction::Added:
        os << "Added";
        break;
    case FileAction::Removed:
        os << "Removed";
        break;
    case FileAction::Modified:
        os << "Modified";
        break;
    case FileAction::RenameOld:
        os << "RenameOld";
        break;
    case FileAction::RenameNew:
        os << "RenameNew";
        break;
    default:
        break;
    }

    return os;
}

void OnFileChanged( FileChangedEventArgs& e )
{
    logger->info( L"File changed: [{}]: {}", e.Action, e.Path );
}