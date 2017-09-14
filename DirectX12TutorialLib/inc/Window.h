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

#include "Object.h"
#include "Events.h"
#include "HighResolutionTimer.h"

 // Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Defined in the Application class.

class DX12TL_DLL Window : public Object
{
public:
    Window(uint32_t width, uint32_t height, const std::wstring& name, bool fullscreen = false, bool vsync = true );
    virtual ~Window();

    // Return the OS window handle.
    HWND GetWindowHandle() const { return m_hWindow; }

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

    // Show the window.
    void Show();

    // Hide the window.
    void Hide();

    const std::wstring& GetWindowTitle() const { return m_Name; }
    void SetWindowTitle(const std::wstring& windowTitle);

    void SetFullscreen(bool fullscreen);
    bool GetFullscreen() const { return m_Fullscreen;  }
    void ToggleFullscreen();

    bool IsVsync() const { return m_VSync; }
    void SetVSync(bool vsync);
    void ToggleVSync();

    // Clear the contents of the window's back buffer
    void Clear(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f);

    // Present the contents of the swapchain backbuffers to the screen.
    void Present();

    // Events on the window:

    // Window is being resized.
    ResizeEvent         Resize;

    // Update 
    UpdateEvent         Update;
    // Render window contents
    RenderEvent         Render;

    // Keyboard events

    // Key was pressed while window has focus.
    KeyboardEvent       KeyPressed;
    // Key was released while window has focus.
    KeyboardEvent       KeyReleased;

    // Mouse events

    // Mouse moved over the window.
    MouseMotionEvent    MouseMoved;
    // Mouse button was pressed over the window.
    MouseButtonEvent    MouseButtonPressed;
    // Mouse button was released over the window.
    MouseButtonEvent    MouseButtonReleased;
    // Mouse wheel was scrolled.
    MouseWheelEvent     MouseWheel;
    // Mouse left the client area.
    Event               MouseLeave;
    // Mouse entered the client area.
    Event               MouseEnter;

    // Event is invoked when the window is closed.
    WindowCloseEvent    Close;

protected:
    // This is required to allow the WndProc function (defined in Application.cpp)
    // to invoke events on the window.
    friend LRESULT CALLBACK ::WndProc(HWND, UINT, WPARAM, LPARAM);

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

    /**
     * Create command list and command allocators.
     */
    virtual void CreateCommandLists();

    /**
     * Resize the swap chain buffers. This is called when the window size 
     * is changed and the swap chain buffers need to be resized to match the 
     * window size.
     */
    virtual void ResizeSwapChainBuffers( uint32_t width, uint32_t height );

    /**
     * Update the render target views for the back buffers of the swap chain.
     * This is done when the swap chain is created or if the swap chain is
     * resized.
     */
    virtual void UpdateSwapChainRenderTargetViews();

    /**
     * Invoked when the game logic should be updated.
     */
    virtual void OnUpdate(UpdateEventArgs& e);

    /**
     * Invoked when the window contents should be rendered.
     */
    virtual void OnRender(RenderEventArgs& e);

    /**
     * Invoked when a keyboard key is pressed while the window has focus.
     */
    virtual void OnKeyPressed(KeyEventArgs& e);
    /**
     * Invoked when a keyboard key is released while the window has focus.
     */
    virtual void OnKeyReleased(KeyEventArgs& e);

    /**
     * Invoked when the mouse moves over the window.
     */
    virtual void OnMouseMoved(MouseMotionEventArgs& e);

    /**
     * Invoked when a mouse button is pressed over the window.
     */
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);

    /**
     * Invoked when a mouse button is released.
     */
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

    /**
     * Invoked when the mouse wheel is scrolled over the window.
     */
    virtual void OnMouseWheel(MouseWheelEventArgs& e);

    /**
     * Invoked when the mouse cursor leaves the client area.
     */
    virtual void OnMouseLeave(EventArgs& e);

    /**
     * Invoked when the mouse enters the client area.
     * This event does not contain the position of the mouse when it entered.
     * Use the OnMouseMoved event to retrieve the position of the mouse.
     */
    virtual void OnMouseEnter(EventArgs& e);

    /**
     * Invoked when the size of the window is changed.
     */
    virtual void OnResize(ResizeEventArgs& e);

    /**
     * Invoked when the window should be closed.
     * Note: The application can cancel the closing event.
     */
    virtual void OnClose(WindowCloseEventArgs& e);

private:
    // In order to receive an event when the mouse leaves the client area,
    // we need to request to receive the WM_MOUSELEAVE event.
    // This function setups up the request to track the mouse leave events.
    void TrackMouseEvents();

    // Number of swap-chain buffers.
    static const uint8_t BufferCount = 2;

    // OS window handle.
    HWND m_hWindow;
    RECT m_WindowRect;

    uint32_t m_Width;
    uint32_t m_Height;
    bool m_Fullscreen;

    // True if using a variable refresh display.
    // (NVidia G-Sync or AMD FreeSync technology).
    bool m_AllowTearing;
    
    std::wstring m_Name;

    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
    // Swap chain back buffers.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[BufferCount];

    // Descriptor heap which holds the Render Target Views for the 
    // back buffers of the swap chain.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    UINT m_RTVDescriptorSize;

    // Command list for clearing / presenting.
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    // One command allocator per frame.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocators[BufferCount];

    // Fence values used to synchronize buffer flipping.
    UINT64 m_FenceValues[BufferCount];

    UINT m_CurrentBackBufferIndex;

    // Timer used to keep track of time since last update.
    HighResolutionTimer m_Timer;
    // Time since the window was created.
    // Used by the Update & Render event args
    double m_TotalTime;
    // Total number of frames since the window was created.
    // Used by the Update & Render event args
    uint64_t m_FrameCounter;

    bool m_IsMinimized;
    bool m_IsMouseInClientArea;
    bool m_VSync;
};