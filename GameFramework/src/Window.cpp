#include "GameFrameworkPCH.h"

#include <GameFramework/Window.h>

Window::Window( HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight )
: m_hWnd( hWnd )
, m_Name( windowName )
, m_Title( windowName )
, m_ClientWidth( clientWidth )
, m_ClientHeight( clientHeight )
, m_PreviousMouseX( 0 )
, m_PreviousMouseY( 0 )
, m_IsFullscreen( false )
, m_IsMinimized( false )
, m_IsMaximized( false )
, m_bInClientRect( false )
, m_bHasKeyboardFocus( false )
{
    m_DPIScaling = ::GetDpiForWindow( hWnd ) / 96.0f;
}

Window::~Window()
{
    ::DestroyWindow( m_hWnd );
}

HWND Window::GetWindowHandle() const
{
    return m_hWnd;
}

float Window::GetDPIScaling() const
{
    return m_DPIScaling;
}

void Window::Show()
{
    ::ShowWindow( m_hWnd, SW_SHOW );
}

void Window::Hide()
{
    ::ShowWindow( m_hWnd, SW_HIDE );
}

void Window::Redraw()
{
    ::RedrawWindow( m_hWnd, nullptr, 0, RDW_INTERNALPAINT );
}

void Window::OnRender( RenderEventArgs& e )
{
    Render( e );
}

void Window::OnClose( WindowCloseEventArgs& e )
{
    Close( e );
}

void Window::OnResize( ResizeEventArgs& e )
{
    m_ClientWidth  = e.Width;
    m_ClientHeight = e.Height;

    if ( ( m_IsMinimized || m_IsMaximized ) && e.State == WindowState::Restored )
    {
        m_IsMaximized = false;
        m_IsMinimized = false;
        OnRestored( e );
    }
    if ( !m_IsMinimized && e.State == WindowState::Minimized )
    {
        m_IsMinimized = true;
        m_IsMaximized = false;
        OnMinimized( e );
    }
    if ( !m_IsMaximized && e.State == WindowState::Maximized )
    {
        m_IsMaximized = true;
        m_IsMinimized = false;
        OnMaximized( e );
    }

    Resize( e );
}

void Window::OnMinimized( ResizeEventArgs& e )
{
    Minimized( e );
}

void Window::OnMaximized( ResizeEventArgs& e )
{
    Maximized( e );
}

void Window::OnRestored( ResizeEventArgs& e )
{
    Restored( e );
}

// The DPI scaling of the window has changed.
void Window::OnDPIScaleChanged( DPIScaleEventArgs& e )
{
    m_DPIScaling = e.DPIScale;
    DPIScaleChanged( e );
}

// A keyboard key was pressed
void Window::OnKeyPressed( KeyEventArgs& e )
{
    KeyPressed( e );
}

// A keyboard key was released
void Window::OnKeyReleased( KeyEventArgs& e )
{
    KeyReleased( e );
}

// Window gained keyboard focus
void Window::OnKeyboardFocus( EventArgs& e )
{
    m_bHasKeyboardFocus = true;
    KeyboardFocus( e );
}

// Window lost keyboard focus
void Window::OnKeyboardBlur( EventArgs& e )
{
    m_bHasKeyboardFocus = false;
    KeyboardBlur( e );
}

// The mouse was moved
void Window::OnMouseMoved( MouseMotionEventArgs& e )
{
    if ( !m_bInClientRect )
    {
        m_PreviousMouseX = e.X;
        m_PreviousMouseY = e.Y;
        m_bInClientRect  = true;
        // Mouse re-entered the client area.
        OnMouseEnter( e );
    }

    e.RelX = e.X - m_PreviousMouseX;
    e.RelY = e.Y - m_PreviousMouseY;

    m_PreviousMouseX = e.X;
    m_PreviousMouseY = e.Y;

    MouseMoved( e );
}

// A button on the mouse was pressed
void Window::OnMouseButtonPressed( MouseButtonEventArgs& e )
{
    MouseButtonPressed( e );
}

// A button on the mouse was released
void Window::OnMouseButtonReleased( MouseButtonEventArgs& e )
{
    MouseButtonReleased( e );
}

void Window::OnMouseWheel( MouseWheelEventArgs& e )
{
    MouseWheel( e );
}

void Window::OnMouseEnter( MouseMotionEventArgs& e )
{
    // Track mouse leave events.
    TRACKMOUSEEVENT trackMouseEvent = {};
    trackMouseEvent.cbSize          = sizeof( TRACKMOUSEEVENT );
    trackMouseEvent.hwndTrack       = m_hWnd;
    trackMouseEvent.dwFlags         = TME_LEAVE;
    TrackMouseEvent( &trackMouseEvent );

    m_bInClientRect = true;
    MouseEnter( e );
}

void Window::OnMouseLeave( EventArgs& e )
{
    m_bInClientRect = false;
    MouseLeave( e );
}

// The window has received mouse focus
void Window::OnMouseFocus( EventArgs& e )
{
    MouseFocus( e );
}

// The window has lost mouse focus
void Window::OnMouseBlur( EventArgs& e )
{
    MouseBlur( e );
}

void Window::SetWindowTitle( const std::wstring& windowTitle )
{
    m_Title = windowTitle;
    ::SetWindowTextW( m_hWnd, m_Title.c_str() );
}

const std::wstring& Window::GetWindowTitle() const
{
    return m_Title;
}

bool Window::IsFullscreen() const
{
    return m_IsFullscreen;
}

// Set the fullscreen state of the window.
void Window::SetFullscreen( bool fullscreen )
{
    if ( m_IsFullscreen != fullscreen )
    {
        m_IsFullscreen = fullscreen;

        if ( m_IsFullscreen )  // Switching to fullscreen.
        {
            // Store the current window dimensions so they can be restored
            // when switching out of fullscreen state.
            ::GetWindowRect( m_hWnd, &m_WindowRect );

            // Set the window style to a borderless fullscreen window so the
            // client area fills the entire screen.
            UINT windowStyle =
                WS_OVERLAPPEDWINDOW & ~( WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX );

            ::SetWindowLongW( m_hWnd, GWL_STYLE, windowStyle );

            // Query the name of the nearest display device for the window.
            // This is required to set the fullscreen dimensions of the window
            // when using a multi-monitor setup.
            HMONITOR      hMonitor    = ::MonitorFromWindow( m_hWnd, MONITOR_DEFAULTTONEAREST );
            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize        = sizeof( MONITORINFOEX );
            ::GetMonitorInfo( hMonitor, &monitorInfo );

            ::SetWindowPos( m_hWnd, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                            SWP_FRAMECHANGED | SWP_NOACTIVATE );

            ::ShowWindow( m_hWnd, SW_MAXIMIZE );
        }
        else
        {
            // Restore all the window decorators.
            ::SetWindowLong( m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW );

            ::SetWindowPos( m_hWnd, HWND_NOTOPMOST, m_WindowRect.left, m_WindowRect.top,
                            m_WindowRect.right - m_WindowRect.left, m_WindowRect.bottom - m_WindowRect.top,
                            SWP_FRAMECHANGED | SWP_NOACTIVATE );

            ::ShowWindow( m_hWnd, SW_NORMAL );
        }
    }
}

void Window::ToggleFullscreen()
{
    SetFullscreen( !m_IsFullscreen );
}
