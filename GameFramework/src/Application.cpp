#include <GameFrameworkPCH.h>
#include <Application.h>

static Application* gs_pSingelton = nullptr;
constexpr wchar_t   WINDOW_CLASS_NAME[] = L"RenderWindowClass";

static LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam,
                                 LPARAM lParam );

Application::Application( HINSTANCE hInst )
: m_hInstance( hInst )
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

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
}

Application& Application::Create( HINSTANCE hInst )
{
    if ( !gs_pSingelton )
    {
        gs_pSingelton = new Application( hInst );
    }

    return *gs_pSingelton;
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
                LOG_WARNING( "Directory change overflow occurred." );
            }
            else
            {
                DWORD        action;
                std::wstring fileName;
                m_DirectoryChanges.Pop( action, fileName );

                FileAction fileAction = FileAction::Unknown;
                switch ( action )
                {
                case FILE_ACTION_ADDED: fileAction = FileAction::Added; break;
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
                default: break;
                }

                FileChangeEventArgs fileChangedEventArgs( *this, fileAction,
                                                          fileName );
                OnFileChange( fileChangedEventArgs );
            }
        default: break;
        }

        std::this_thread::yield();
    }
}

void Application::OnFileChange( FileChangeEventArgs& e )
{
    FileChanged( e );
}

