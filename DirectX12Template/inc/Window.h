/*
 *  Copyright(c) 2017 Jeremiah van Oosten
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
 *  @date August 28, 2017
 *  @author Jeremiah van Oosten
 *
 *  @brief Window class for DirectX12 Application.
 */

#pragma once

#include "DirectX12TemplateDefines.h"

class DX12TL_DLL Window
{
public:
    Window(uint32_t width, uint32_t height, const std::wstring& name, bool fullscreen = false );
    virtual ~Window();

    // Return the OS window handle.
    HWND GetWindowHandle() const { return m_hWindow; }

    // Show the window.
    void Show();

    // Hide the window.
    void Hide();

protected:
    
    /**
     * Get the window class info. Override this for your own classes to change the
     * way windows created with this class appear.
     */
    virtual WNDCLASSEXW GetWindowClassInfo(HINSTANCE hInst) const;

    /**
     * Create the actual window. Override this function to change the window style.
     */
    virtual void CreateWindow();

    /**
     * Create the swap chain for the window.
     */
    virtual void CreateSwapChain();

private:
    // OS window handle.
    HWND m_hWindow;

    uint32_t m_Width;
    uint32_t m_Height;
    bool m_Fullscreen;

    std::wstring m_Name;
};