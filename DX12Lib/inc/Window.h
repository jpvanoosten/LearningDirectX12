/**
* @brief A window for our application.
*/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Events.h>
#include <HighResolutionClock.h>

// Forward-declare the DirectXTemplate class.
class Game;

class Window
{
public:

    /**
    * Get a handle to this window's instance.
    * @returns The handle to the window instance or nullptr if this is not a valid window.
    */
    HWND get_WindowHandle() const;

    /**
    * Destroy this window.
    */
    void Destroy();

    const std::wstring& get_WindowName() const;

    int get_ClientWidth() const;
    int get_ClientHeight() const;

    /**
    * Should this window be rendered with vertical refresh synchronization.
    */
    bool get_VSync() const;

    /**
    * Is this a windowed window or full-screen?
    */
    bool get_Windowed() const;

    /**
     * Show this window.
     */
    void Show();

    /**
     * Hide the window.
     */
    void Hide();

protected:
    // The Window procedure needs to call protected methods of this class.
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // Only the application can create a window.
    friend class Application;
    // The DirectXTemplate class needs to register itself with a window.
    friend class Game;

    Window() = delete;
    Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync );
    virtual ~Window();

    // Register a Game with this window. This allows
    // the window to callback functions in the Game class and notify 
    // the demo that the window has been destroyed.
    void RegisterCallbacks( std::shared_ptr<Game> pGame );

    // Update and Draw can only be called by the application.
    virtual void OnUpdate(UpdateEventArgs& e);
    virtual void OnRender(RenderEventArgs& e);

    // A keyboard key was pressed
    virtual void OnKeyPressed(KeyEventArgs& e);
    // A keyboard key was released
    virtual void OnKeyReleased(KeyEventArgs& e);

    // The mouse was moved
    virtual void OnMouseMoved(MouseMotionEventArgs& e);
    // A button on the mouse was pressed
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
    // A button on the mouse was released
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
    // The mouse wheel was moved.
    virtual void OnMouseWheel(MouseWheelEventArgs& e);

    // The window was resized.
    virtual void OnResize(ResizeEventArgs& e);

private:
    // Windows should not be copied.
    Window(const Window& copy) = delete;

    HWND m_hWnd;

    std::wstring m_WindowName;
    
    int m_ClientWidth;
    int m_ClientHeight;
    bool m_VSync;
    bool m_bWindowed;

    HighResolutionClock m_UpdateClock;
    HighResolutionClock m_RenderClock;

    uint64_t m_FrameCount;

    std::weak_ptr<Game> m_pGame;
};