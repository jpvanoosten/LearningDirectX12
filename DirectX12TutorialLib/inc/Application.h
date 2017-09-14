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
 *  @file Application.h
 *  @date August 28, 2017
 *  @author Jeremiah van Oosten
 *
 *  @brief Application class for DirectX 12 template.
 */

#pragma once

#include "Object.h"
#include "Events.h"

// Undefine the CreateWindow macro so I can use a function with the same name in this class.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Forward declarations.
class Window;
class Game;

using AdapterList = std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>>;

class DX12TL_DLL Application : public Object
{
public:
    Application( HINSTANCE hInstance, int argc, const wchar_t* argv[] );
    virtual ~Application();

    static Application& Get();

    HINSTANCE GetInstanceHandle() const { return m_hInstance;  }

    // Run until the application quits.
    virtual int Run();

    // Close all windows and stop the application.
    virtual void Stop();

    // Creates a window.
    // The application needs to keep track of windows to know how
    // to forward events to the appropriate window.
    virtual std::shared_ptr<Window> CreateWindow(uint32_t width, uint32_t height, 
        const std::wstring& name, 
        bool fullscreen = false,
        bool vsync = true
    );

    // Retrieve the DirectX 12 device owned by the application.
    Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const { return m_Device;  }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

    // Signal the command queue and return the fence value to wait for.
    UINT64 Signal(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

    // Get the currently completed fence value.
    UINT64 GetCompletedFenceValue() const;
    // Check to see if the fence has reached a specific fence value.
    bool IsFenceComplete(UINT64 fenceValue) const;

    // Wait for the GPU to reach a particular fence value.
    void WaitForFenceValue(UINT64 fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max() );

    // Wait for all command queues to finish.
    // Before any resources can be released, all GPU commands referencing
    // those resources must be finished.
    void WaitForGPU();

    // Check to see if the display supports tearing.
    // This is required for variable refresh rate displays
    // (Nvidia G-Sync or AMD FreeSync).
    bool AllowTearing() const { return m_bAllowTearing; }

protected:
    // Retrieve a list of DirectX12 adapters.
    virtual AdapterList GetAdapters(bool useWarp = false) const;

    // Creates a DirectX device from the specified adapter.
    virtual Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);

    // Creates command queues.
    virtual void CreateCommandQueues(Microsoft::WRL::ComPtr<ID3D12Device2> device);
    virtual Microsoft::WRL::ComPtr<ID3D12CommandQueue> CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, INT priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE, UINT nodeMask = 0);

    // Check to see the if the display supports variable refresh rate.
    bool CheckTearingSupport();

private:
    // Non copyable.
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;

    // Handle to the instance of the application.
    // Passed in from the main entry point.
    HINSTANCE m_hInstance;

    // Direct3D device.
    Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
    // Direct, Compute, and Copy command queues.
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_GraphicsCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_ComputeCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CopyCommandQueue;

    // Synchronization objects
    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    HANDLE m_FenceEvent;

    // Use a single fence value for all command queues.
    // This should be fine as long as the fence value only increases.
    std::atomic_uint64_t m_FenceValue;

    bool m_Quit;

    // Set to true to use a WARP adapter.
    bool m_bUseWarp;

    // Allow screen tearing on displays that support variable refresh rates.
    // (NVidia G-Sync, or AMD FreeSync).
    bool m_bAllowTearing;
};