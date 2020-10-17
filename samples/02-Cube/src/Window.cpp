#include <Window.h>

#include <Application.h>
#include <CommandQueue.h>
#include <Game.h>
#include <Helpers.h>

#include <d3dx12.h>

#include <algorithm>
#include <cassert>

using namespace Microsoft::WRL;

Window::Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync )
    : m_hWnd(hWnd)
    , m_WindowName(windowName)
    , m_ClientWidth(clientWidth)
    , m_ClientHeight(clientHeight)
    , m_VSync(vSync)
    , m_Fullscreen(false)
    , m_FrameCounter(0)
{
    Application& app = Application::Get();

    m_IsTearingSupported = app.IsTearingSupported();

    m_dxgiSwapChain = CreateSwapChain();
    m_d3d12RTVDescriptorHeap = app.CreateDescriptorHeap(BufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_RTVDescriptorSize = app.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    UpdateRenderTargetViews();
}

Window::~Window()
{
    // Window should be destroyed with Application::DestroyWindow before
    // the window goes out of scope.
    assert(!m_hWnd && "Use Application::DestroyWindow before destruction.");
}

HWND Window::GetWindowHandle() const
{
    return m_hWnd;
}

const std::wstring& Window::GetWindowName() const
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

    // Remove back buffer resource from the global resource state tracker.
    for (int i = 0; i < BufferCount; ++i)
    {
        auto resource = m_d3d12BackBuffers[i].Get();
        m_d3d12BackBuffers[i].Reset();
    }

    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

int Window::GetClientWidth() const
{
    return m_ClientWidth;
}

int Window::GetClientHeight() const
{
    return m_ClientHeight;
}

bool Window::IsVSync() const
{
    return m_VSync;
}

void Window::SetVSync(bool vSync)
{
    m_VSync = vSync;
}

void Window::ToggleVSync()
{
    SetVSync(!m_VSync);
}

bool Window::IsFullScreen() const
{
    return m_Fullscreen;
}

// Set the fullscreen state of the window.
void Window::SetFullscreen(bool fullscreen)
{
    if (m_Fullscreen != fullscreen)
    {
        m_Fullscreen = fullscreen;

        if (m_Fullscreen) // Switching to fullscreen.
        {
            // Store the current window dimensions so they can be restored 
            // when switching out of fullscreen state.
            ::GetWindowRect(m_hWnd, &m_WindowRect);

            // Set the window style to a borderless window so the client area fills
            // the entire screen.
            UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

            ::SetWindowLongW(m_hWnd, GWL_STYLE, windowStyle);

            // Query the name of the nearest display device for the window.
            // This is required to set the fullscreen dimensions of the window
            // when using a multi-monitor setup.
            HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            ::GetMonitorInfo(hMonitor, &monitorInfo);

            ::SetWindowPos(m_hWnd, HWND_TOP,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(m_hWnd, SW_MAXIMIZE);
        }
        else
        {
            // Restore all the window decorators.
            ::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

            ::SetWindowPos(m_hWnd, HWND_NOTOPMOST,
                m_WindowRect.left,
                m_WindowRect.top,
                m_WindowRect.right - m_WindowRect.left,
                m_WindowRect.bottom - m_WindowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(m_hWnd, SW_NORMAL);
        }
    }
}

void Window::ToggleFullscreen()
{
    SetFullscreen(!m_Fullscreen);
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
        m_FrameCounter++;

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
    if (m_ClientWidth != e.Width || m_ClientHeight != e.Height)
    {
        m_ClientWidth = std::max(1, e.Width);
        m_ClientHeight = std::max(1, e.Height);

        Application::Get().Flush();

        for (int i = 0; i < BufferCount; ++i)
        {
            m_d3d12BackBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(m_dxgiSwapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(m_dxgiSwapChain->ResizeBuffers(BufferCount, m_ClientWidth,
            m_ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        m_CurrentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews();
    }

    if (auto pGame = m_pGame.lock())
    {
        pGame->OnResize(e);
    }
}

Microsoft::WRL::ComPtr<IDXGISwapChain4> Window::CreateSwapChain()
{
    Application& app = Application::Get();

    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_ClientWidth;
    swapChainDesc.Height = m_ClientHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = m_IsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    ID3D12CommandQueue* pCommandQueue = app.GetCommandQueue()->GetD3D12CommandQueue().Get();

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        pCommandQueue,
        m_hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

    m_CurrentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

    return dxgiSwapChain4;
}

// Update the render target views for the swapchain back buffers.
void Window::UpdateRenderTargetViews()
{
    auto device = Application::Get().GetDevice();

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < BufferCount; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        m_d3d12BackBuffers[i] = backBuffer;

        rtvHandle.Offset(m_RTVDescriptorSize);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Window::GetCurrentRenderTargetView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrentBackBufferIndex, m_RTVDescriptorSize);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Window::GetCurrentBackBuffer() const
{
    return m_d3d12BackBuffers[m_CurrentBackBufferIndex];
}

UINT Window::GetCurrentBackBufferIndex() const
{
    return m_CurrentBackBufferIndex;
}

UINT Window::Present()
{
    UINT syncInterval = m_VSync ? 1 : 0;
    UINT presentFlags = m_IsTearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(m_dxgiSwapChain->Present(syncInterval, presentFlags));
    m_CurrentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

    return m_CurrentBackBufferIndex;
}
