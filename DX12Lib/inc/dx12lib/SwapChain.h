#pragma once

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
 *  @file SwapChain.h
 *  @date October 12, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Wrapper for a IDXGISwapChain.
 */

// #include <dx12lib/GUI.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/Texture.h>

#include <dxgi1_5.h>     // For IDXGISwapChain4
#include <wrl/client.h>  // For Microsoft::WRL::ComPtr

#include <memory>  // For std::shared_ptr

namespace dx12lib
{

class CommandQueue;
class Device;

class SwapChain
{
public:
    // Number of swapchain back buffers.
    static const UINT BufferCount = 3;

    /**
     * Check to see if the swap chain is in full-screen exclusive mode.
     */
    bool IsFullscreen() const
    {
        return m_Fullscreen;
    }

    /**
     * Set the swap chain to fullscreen exclusive mode (true) or windowed mode (false).
     */
    void SetFullscreen( bool fullscreen );

    /**
     * Toggle fullscreen exclusive mode.
     */
    void ToggleFullscreen()
    {
        SetFullscreen( !m_Fullscreen );
    }

    /**
     * Check to see if tearing is supported.
     *
     * @see https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays
     */
    bool IsTearingSupported() const
    {
        return m_TearingSupported;
    }

    /**
     * Block the current thread until the swapchain has finished presenting.
     * Doing this at the beginning of the update loop can improve input latency.
     */
    void WaitForSwapChain();


    /**
     * Resize the swapchain's back buffers. This should be called whenever the window is resized.
     */
    void Resize( uint32_t width, uint32_t height );

    /**
     * Get the render target of the window. This method should be called every
     * frame since the color attachment point changes depending on the window's
     * current back buffer.
     */
    const RenderTarget& GetRenderTarget() const;

    /**
     * Present the swapchain's back buffer to the screen.
     *
     * @param texture The texture to copy to the swap chain's backbuffer before
     * presenting. By default, this is an empty texture. In this case, no copy
     * will be performed. Use the SwapChain::GetRenderTarget method to get a render
     * target for the window's color buffer.
     *
     * @returns The current backbuffer index after the present.
     */
    UINT Present( const Texture& texture = Texture() );

    Microsoft::WRL::ComPtr<IDXGISwapChain4> GetDXGISwapChain() const;

protected:
    // Swap chains can only be created through the Device.
    SwapChain( std::shared_ptr<Device> device, HWND hWnd );
    virtual ~SwapChain();

    // Update the swapchain's RTVs.
    void UpdateRenderTargetViews();

private:
    // The device that was used to create the SwapChain.
    std::weak_ptr<Device> m_Device;
    // The command queue that is used to create the swapchain.
    // The command queue will be signaled right after the Present
    // to ensure that the swapchain's back buffers are not in-flight before
    // the next frame is allowed to be rendered.
    std::shared_ptr<CommandQueue>           m_CommandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
    Texture                                 m_BackBufferTextures[BufferCount];
    mutable RenderTarget                    m_RenderTarget;

    //GUI m_GUI;

    // The current backbuffer index of the swap chain.
    UINT   m_CurrentBackBufferIndex;
    UINT64 m_FenceValues[BufferCount];  // The fence values to wait for before leaving the Present method.

    // A handle to a waitable object. Used to wait for the swapchain before presenting.
    HANDLE m_hFrameLatencyWaitableObject;

    // The window handle that is associates with this swap chain.
    HWND m_hWnd;

    // The current width/height of the swap chain.
    uint32_t m_Width;
    uint32_t m_Height;

    // Should presentation be synchronized with the vertical refresh rate of the screen?
    // Set to true to eliminate screen tearing.
    bool m_VSync;

    // Whether or not tearing is supported.
    bool m_TearingSupported;

    // Whether the application is in full-screen exclusive mode or windowed mode.
    bool m_Fullscreen;
};

}  // namespace dx12lib
