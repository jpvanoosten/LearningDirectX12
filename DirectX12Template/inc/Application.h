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

#include "DirectX12TemplateDefines.h"

// Undefine the CreateWindow macro so I can use a function with the same name in this class.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Forward declarations.
class Window;
class Game;

using AdapterList = std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>>;

class DX12TL_DLL Application
{
public:
    Application( HINSTANCE hInstance, int argc, const wchar_t* argv[] );
    virtual ~Application();

    static Application& Get();

    HINSTANCE GetInstanceHandle() const { return m_hInstance;  }

    // Run until the application quits.
    virtual int Run( Game& game );

    // Close all windows and stop the application.
    virtual void Stop();

    // Creates a window.
    // The application needs to keep track of windows to know how
    // to forward events to the appropriate window.
    virtual std::shared_ptr<Window> CreateWindow(uint32_t width, uint32_t height, const std::wstring& name, 
                                                 bool fullscreen = true );

    // Retrieve the DirectX 12 device.
    Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const { return m_Device;  }

protected:

    // Retrieve a list of DirectX12 adapters.
    virtual AdapterList GetAdapters(bool useWarp = false) const;

    // Creates a DirectX device from the specified adapter.
    virtual Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);


private:
    // Non copyable.
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;

    // Handle to the instance of the application.
    // Passed in from the main entry point.
    HINSTANCE m_hInstance;

    // Direct3D device.
    Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;

    bool m_Quit;

    // Set to true to use a WARP adapter.
    bool m_bUseWarp;
};