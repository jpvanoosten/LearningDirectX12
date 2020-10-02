#include <GameFrameworkPCH.h>
#include <Window.h>

Window::Window( HWND hWnd, const std::wstring& windowName, int clientWidth,
                int clientHeight )
: m_hWnd( hWnd )
, m_ClientWidth( clientWidth )
, m_ClientHeight( clientHeight )
, m_PreviousMouseX( 0 )
, m_PreviousMouseY( 0 )
, m_IsFullscreen( false )
, m_bInClientRect( false )
, m_bHasKeyboardFocus( false )
{}

void Window::Show()
{
    ::ShowWindow( m_hWnd, SW_SHOW );
}

void Window::Hide()
{
    ::ShowWindow( m_hWnd, SW_HIDE );
}

void Window::OnClose( WindowCloseEventArgs& e )
{
    Close( e );
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