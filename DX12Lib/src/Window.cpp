#include <DX12LibPCH.h>
#include <Window.h>
#include <Game.h>

Window::Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync )
    : m_hWnd(hWnd)
    , m_WindowName(windowName)
    , m_ClientWidth(clientWidth)
    , m_ClientHeight(clientHeight)
    , m_VSync(vSync)
    , m_bWindowed(false)
    , m_FrameCount(0)
{
}

Window::~Window()
{
    Destroy();
}

HWND Window::get_WindowHandle() const
{
    return m_hWnd;
}

const std::wstring& Window::get_WindowName() const
{
    return m_WindowName;
}

void Window::Show()
{
    ::ShowWindow(m_hWnd, SW_SHOW);
}

/**
* Hide the window.
*/
void Window::Hide()
{
    ::ShowWindow(m_hWnd, SW_HIDE);
}

void Window::Destroy()
{
    if (auto pGame = m_pGame.lock())
    {
        // Notify the registered game that the window is being destroyed.
        pGame->OnWindowDestroy();
    }
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

int Window::get_ClientWidth() const
{
    return m_ClientWidth;
}

int Window::get_ClientHeight() const
{
    return m_ClientHeight;
}

bool Window::get_VSync() const
{
    return m_VSync;
}

bool Window::get_Windowed() const
{
    return m_bWindowed;
}

void Window::RegisterCallbacks(std::shared_ptr<Game> pGame)
{
    m_pGame = pGame;

    return;
}

void Window::OnUpdate(UpdateEventArgs&)
{
    m_UpdateClock.Tick();

    if (auto pGame = m_pGame.lock())
    {
        m_FrameCount++;

        UpdateEventArgs updateEventArgs(m_UpdateClock.GetDeltaSeconds(), m_UpdateClock.GetTotalSeconds());
        pGame->OnUpdate(updateEventArgs);
    }
}

void Window::OnRender(RenderEventArgs&)
{
    m_RenderClock.Tick();

    if (auto pGame = m_pGame.lock())
    {
        RenderEventArgs renderEventArgs(m_RenderClock.GetDeltaSeconds(), m_RenderClock.GetTotalSeconds());
        pGame->OnRender(renderEventArgs);
    }
}

void Window::OnKeyPressed(KeyEventArgs& e)
{
    if (auto pGame = m_pGame.lock())
    {
        pGame->OnKeyPressed(e);
    }
}

void Window::OnKeyReleased(KeyEventArgs& e)
{
    if (auto pGame = m_pGame.lock())
    {
        pGame->OnKeyReleased(e);
    }
}

// The mouse was moved
void Window::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (auto pGame = m_pGame.lock())
    {
        pGame->OnMouseMoved(e);
    }
}

// A button on the mouse was pressed
void Window::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    if (auto pGame = m_pGame.lock())
    {
        pGame->OnMouseButtonPressed(e);
    }
}

// A button on the mouse was released
void Window::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    if (auto pGame = m_pGame.lock())
    {
        pGame->OnMouseButtonReleased(e);
    }
}

// The mouse wheel was moved.
void Window::OnMouseWheel(MouseWheelEventArgs& e)
{
    if (auto pGame = m_pGame.lock())
    {
        pGame->OnMouseWheel(e);
    }
}

void Window::OnResize(ResizeEventArgs& e)
{
    // Update the client size.
    if (e.Width <= 0) e.Width = 1;
    if (e.Height <= 0) e.Height = 1;

    m_ClientWidth = e.Width;
    m_ClientHeight = e.Height;

    if (auto pGame = m_pGame.lock())
    {
        pGame->OnResize(e);
    }
}
