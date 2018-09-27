#include <DX12LibPCH.h>

#include <Window.h>

#include <Application.h>
#include <CommandQueue.h>
#include <CommandList.h>
#include <Game.h>
#include <GUI.h>
#include <RenderTarget.h>
#include <ResourceStateTracker.h>
#include <Texture.h>

Window::Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync )
    : m_hWnd(hWnd)
    , m_WindowName(windowName)
    , m_ClientWidth(clientWidth)
    , m_ClientHeight(clientHeight)
    , m_VSync(vSync)
    , m_Fullscreen(false)
    , m_FenceValues{0}
    , m_FrameValues{0}
{
    Application& app = Application::Get();

    m_IsTearingSupported = app.IsTearingSupported();

    for (int i = 0; i < BufferCount; ++i)
    {
        m_BackBufferTextures[i].SetName(L"Backbuffer[" + std::to_wstring(i) + L"]" );
    }

    m_dxgiSwapChain = CreateSwapChain();
    UpdateRenderTargetViews();
}

Window::~Window()
{
    // Window should be destroyed with Application::DestroyWindow before
    // the window goes out of scope.
    assert(!m_hWnd && "Use Application::DestroyWindow before destruction.");
}

void Window::Initialize()
{
    m_GUI.Initialize( shared_from_this() );
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
    m_GUI.Destroy();

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

void Window::OnUpdate(UpdateEventArgs& e)
{
    m_GUI.NewFrame();

    m_UpdateClock.Tick();

    if (auto pGame = m_pGame.lock())
    {
        UpdateEventArgs updateEventArgs(m_UpdateClock.GetDeltaSeconds(), m_UpdateClock.GetTotalSeconds(), e.FrameNumber);
        pGame->OnUpdate(updateEventArgs);
    }
}

void Window::OnRender(RenderEventArgs& e)
{
    m_RenderClock.Tick();

    if (auto pGame = m_pGame.lock())
    {
        RenderEventArgs renderEventArgs(m_RenderClock.GetDeltaSeconds(), m_RenderClock.GetTotalSeconds(), e.FrameNumber);
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
    e.RelX = e.X - m_PreviousMouseX;
    e.RelY = e.Y - m_PreviousMouseY;

    m_PreviousMouseX = e.X;
    m_PreviousMouseY = e.Y;

    if (auto pGame = m_pGame.lock())
    {
        pGame->OnMouseMoved(e);
    }
}

// A button on the mouse was pressed
void Window::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    m_PreviousMouseX = e.X;
    m_PreviousMouseY = e.Y;

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

        // Release all references to back buffer textures.
        m_RenderTarget.AttachTexture( Color0, Texture() );
        for (int i = 0; i < BufferCount; ++i)
        {
            ResourceStateTracker::RemoveGlobalResourceState(m_BackBufferTextures[i].GetD3D12Resource().Get());
            m_BackBufferTextures[i].Reset();
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

void Window::UpdateRenderTargetViews()
{
    for (int i = 0; i < BufferCount; ++i )
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
        
        ResourceStateTracker::AddGlobalResourceState(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);

        m_BackBufferTextures[i].SetD3D12Resource(backBuffer);
        m_BackBufferTextures[i].CreateViews();
    }
}

const RenderTarget& Window::GetRenderTarget() const
{
    m_RenderTarget.AttachTexture( AttachmentPoint::Color0, m_BackBufferTextures[m_CurrentBackBufferIndex] );
    return m_RenderTarget;
}

UINT Window::Present( const Texture& texture )
{
    auto commandQueue = Application::Get().GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto commandList = commandQueue->GetCommandList();

    auto& backBuffer = m_BackBufferTextures[m_CurrentBackBufferIndex];

    if( texture.IsValid() )
    {
        if ( texture.GetD3D12ResourceDesc().SampleDesc.Count > 1 )
        {
            commandList->ResolveSubresource( backBuffer, texture );
        }
        else
        {
            commandList->CopyResource( backBuffer, texture );
        }
    }

    RenderTarget renderTarget;
    renderTarget.AttachTexture( AttachmentPoint::Color0, backBuffer );

    m_GUI.Render( commandList, renderTarget );

    commandList->TransitionBarrier( backBuffer, D3D12_RESOURCE_STATE_PRESENT );
    commandQueue->ExecuteCommandList( commandList );

    UINT syncInterval = m_VSync ? 1 : 0;
    UINT presentFlags = m_IsTearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(m_dxgiSwapChain->Present(syncInterval, presentFlags));

    m_FenceValues[m_CurrentBackBufferIndex] = commandQueue->Signal();
    m_FrameValues[m_CurrentBackBufferIndex] = Application::GetFrameCount();

    m_CurrentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

    commandQueue->WaitForFenceValue( m_FenceValues[m_CurrentBackBufferIndex] );

    Application::Get().ReleaseStaleDescriptors( m_FrameValues[m_CurrentBackBufferIndex] );

    return m_CurrentBackBufferIndex;
}
