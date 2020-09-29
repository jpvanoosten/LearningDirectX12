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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <string>

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
     * Window close event is fired when the window is about to be closed.
     */
    WindowCloseEvent WindowClose;

    /**
     * Invoked when the window is minimized.
     */
    Event Minimize;

    /**
     * Invoked when the window is restored.
     */
    Event Restore;

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

    /**
     * Invoked when the window is resized.
     */
    ResizeEvent Resize;

protected:
private:
};
