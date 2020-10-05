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
 *  @file Window.h
 *  @date September 29, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief A Window class for the GameFramework.
 */

#include "Events.h"
#include "HighResolutionTimer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <string>

// Forward declarations
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM,
                          LPARAM );  // Defined in the Application class.

class Window
{
public:
    /**
     * Get a handle to the OS window instance.
     *
     * @returns The handle to the OS window instance.
     */
    HWND GetWindowHandle() const;

    /**
     * Get the current (normalized) DPI scaling for this window.
     *
     * @returns The (normalized) DPI scaling for this window.
     */
    float GetDPIScaling() const;

    /**
     * Get the name that was used to create the window.
     *
     * @returns The name that was used to create the window.
     */
    const std::wstring& GetWindowName() const;

    /**
     * Set the window title.
     *
     * @param title The new title of the window.
     */
    void SetWindowTitle( const std::wstring& windowTitle );

    /**
     * Get the current title of the window.
     *
     * @returns The current title of the window.
     */
    const std::wstring& GetWindowTitle() const;

    /**
     * Get the width of the window's client area.
     *
     * @returns The width of the window's client area (in pixels).
     */
    int GetClientWidth() const;

    /**
     * Get the height of the window's client area.
     *
     * @returns The height of the window's client area (in pixels).
     */
    int GetClientHeight() const;

    /**
     * Check to see if this window is in fullscreen mode.
     */
    bool IsFullscreen() const;

    /**
     * Set the window's fullscreen state.
     *
     * @param fullscreen true to set the window to fullscreen, false to set to
     * windowed mode.
     */
    void SetFullscreen( bool fullscreen );

    /**
     * Toggle the current fullscreen state of the window.
     */
    void ToggleFullscreen();

    /**
     * Show the window.
     */
    void Show();

    /**
     * Hide the window.
     */
    void Hide();

    /**
     * Invoked when the game should be updated.
     */
    UpdateEvent Update;

    /**
     * The DPI scaling of the window has changed.
     */
    DPIScaleEvent DPIScaleChanged;

    /**
     * Window close event is fired when the window is about to be closed.
     */
    WindowCloseEvent Close;

    /**
     * Invoked when the window is resized.
     */
    ResizeEvent Resize;

    /**
     * Invoked when the window is minimized.
     */
    ResizeEvent Minimized;

    /**
     * Invoked when the window is maximized.
     */
    ResizeEvent Maximized;

    /**
     * Invoked when the window is restored.
     */
    ResizeEvent Restored;

    /**
     * Invoked when a keyboard key is pressed while the window has focus.
     */
    KeyboardEvent KeyPressed;

    /**
     * Invoked when a keyboard key is released while the window has focus.
     */
    KeyboardEvent KeyReleased;

    /**
     * Invoked when the window gains keyboard focus.
     */
    Event KeyboardFocus;

    /**
     * Invoked when the window loses keyboard focus.
     */
    Event KeyboardBlur;

    /**
     * Invoked when the mouse is moved over the window.
     */
    MouseMotionEvent MouseMoved;

    /**
     * Invoked when the mouse enters the client area.
     */
    MouseMotionEvent MouseEnter;

    /**
     * Invoked when the mouse button is pressed over the window.
     */
    MouseButtonEvent MouseButtonPressed;

    /**
     * Invoked when the mouse button is released over the window.
     */
    MouseButtonEvent MouseButtonReleased;

    /**
     * Invoked when the mouse wheel is scrolled over the window.
     */
    MouseWheelEvent MouseWheel;

    /**
     * Invoked when the mouse cursor leaves the client area.
     */
    Event MouseLeave;

    /**
     * Invoked when the window gains mouse focus.
     */
    Event MouseFocus;

    /**
     * Invoked when the window looses mouse focus.
     */
    Event MouseBlur;

protected:
    friend class Application;
    // This is needed to allow the WndProc function to call event callbacks on
    // the window.
    friend LRESULT CALLBACK ::WndProc( HWND, UINT, WPARAM, LPARAM );

    // Only the Application class can create Windows.
    Window( HWND hWnd, const std::wstring& windowName, int clientWidth,
            int clientHeight );

    ~Window();

    // Update game
    virtual void OnUpdate( UpdateEventArgs& e );

    // The DPI scaling of the window has changed.
    virtual void OnDPIScaleChanged( DPIScaleEventArgs& e );

    // Window was closed.
    virtual void OnClose( WindowCloseEventArgs& e );

    // Window was resized.
    virtual void OnResize( ResizeEventArgs& e );

    // Window was minimized.
    virtual void OnMinimized( ResizeEventArgs& e );

    // Window was maximized.
    virtual void OnMaximized( ResizeEventArgs& e );

    // Window was restored.
    virtual void OnRestored( ResizeEventArgs& e );

    // A keyboard key was pressed.
    virtual void OnKeyPressed( KeyEventArgs& e );
    // A keyboard key was released
    virtual void OnKeyReleased( KeyEventArgs& e );
    // Window gained keyboard focus
    virtual void OnKeyboardFocus( EventArgs& e );
    // Window lost keyboard focus
    virtual void OnKeyboardBlur( EventArgs& e );

    // The mouse was moved
    virtual void OnMouseMoved( MouseMotionEventArgs& e );
    // A button on the mouse was pressed
    virtual void OnMouseButtonPressed( MouseButtonEventArgs& e );
    // A button on the mouse was released
    virtual void OnMouseButtonReleased( MouseButtonEventArgs& e );
    // The mouse wheel was moved.
    virtual void OnMouseWheel( MouseWheelEventArgs& e );

    // The mouse entered the client area.
    virtual void OnMouseEnter( MouseMotionEventArgs& e );
    // The mouse left the client are of the window.
    virtual void OnMouseLeave( EventArgs& e );
    // The application window has received mouse focus
    virtual void OnMouseFocus( EventArgs& e );
    // The application window has lost mouse focus
    virtual void OnMouseBlur( EventArgs& e );

private:
    HWND m_hWnd;

    std::string m_Name;

    uint32_t m_ClientWidth;
    uint32_t m_ClientHeight;

    int32_t m_PreviousMouseX;
    int32_t m_PreviousMouseY;

    // The current fullscreen state of the window.
    bool m_IsFullscreen;

    // True if the window is maximized.
    bool m_IsMaximized;

    // True if the window is minimized.
    bool m_IsMinimized;

    // This is true when the mouse is inside the window's client rect.
    bool m_bInClientRect;
    RECT m_WindowRect;

    // This is set to true when the window receives keyboard focus.
    bool m_bHasKeyboardFocus;

    HighResolutionTimer m_Timer;
};
