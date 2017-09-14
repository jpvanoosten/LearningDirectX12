#include <DirectX12TutorialPCH.h>

#include <Application.h>
#include <Helpers.h>

#include <Window.h>

// Import namespaces.
using namespace Microsoft::WRL;

// Constants
constexpr auto WINDOW_CLASS_NAME = L"DX12WindowClass";

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Defined in the Application class.

Window::Window(uint32_t width, uint32_t height, const std::wstring& name, bool fullscreen, bool vsync )
    : m_Width(width)
    , m_Height(height)
    , m_Fullscreen(false)
    , m_Name(name)
    , m_FenceValues{}
    , m_CurrentBackBufferIndex(0)
    , m_IsMinimized(false)
    , m_IsMouseInClientArea(false)
    , m_VSync( vsync )
{
    // Check to see if the monitor supports variable refresh rates.
    m_AllowTearing = Application::Get().AllowTearing();

    // Create the descriptor heap for the render target views for the back buffers
    // of the swap chain.
    auto device = Application::Get().GetDevice();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVDescriptorHeap)));

    // Sizes of descriptors is vendor specific and must be queried at runtime.
    m_RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CreateWindow();
    CreateSwapChain();
    CreateCommandLists();

    SetFullscreen(fullscreen);
}

Window::~Window()
{
    // Wait for all commands on the GPU to finish before we release
    // any GPU resources.
    Application::Get().WaitForGPU();

    ::DestroyWindow(m_hWindow);
}

WNDCLASSEXW Window::GetWindowClassInfo(HINSTANCE hInst) const
{
    // Register a window class for creating our render windows with.
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInst;
    windowClass.hIcon = ::LoadIcon(hInst, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hIconSm = ::LoadIcon(hInst, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));

    return windowClass;
}

void Window::CreateWindow()
{
    HINSTANCE hInstance = Application::Get().GetInstanceHandle();

    // Store the result in a local static to ensure this function is called only
    // once when the first window is created.
    static HRESULT hr = ::RegisterClassExW(&GetWindowClassInfo(hInstance));
    assert(SUCCEEDED(hr));

    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    m_hWindow = ::CreateWindowExW(
        NULL,
        WINDOW_CLASS_NAME,
        m_Name.c_str(),
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    assert(m_hWindow && "Failed to create window");

    ::GetWindowRect(m_hWindow, &m_WindowRect);

    ::SetWindowTextW(m_hWindow, m_Name.c_str());
}

void Window::CreateCommandLists()
{
    auto device = Application::Get().GetDevice();

    for (int i = 0; i < FrameCount; ++i)
    {
        ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i])));
    }

    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_CurrentBackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
    ThrowIfFailed(m_CommandList->Close());
}

void Window::CreateSwapChain()
{
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    // Get the direct command queue from the application instance.
    // This is required to create the swap chain.
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    // Make sure all GPU commands are finished before (re) creating the swap chain for this window.
    Application::Get().WaitForGPU();

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_Width;
    swapChainDesc.Height = m_Height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        m_hWindow,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    dxgiFactory4->MakeWindowAssociation(m_hWindow, DXGI_MWA_NO_ALT_ENTER);

    ThrowIfFailed(swapChain1.As(&m_SwapChain));
    
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    UpdateSwapChainRenderTargetViews();
}


void Window::ResizeSwapChainBuffers(uint32_t width, uint32_t height)
{
    if (m_Width != width || m_Height != height)
    {
        m_Width = width;
        m_Height = height;

        // Stall the CPU until the GPU is finished with any queued render
        // commands. This is required before we can resize the swap chain buffers.
        Application::Get().WaitForGPU();

        // Before the buffers can be resized, all references to those buffers
        // need to be released.
        for (int i = 0; i < FrameCount; ++i)
        {
            m_BackBuffers[i].Reset();
            m_FenceValues[i] = m_FenceValues[m_CurrentBackBufferIndex];
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(m_SwapChain->ResizeBuffers(FrameCount,
            m_Width, m_Height,
            swapChainDesc.BufferDesc.Format,
            swapChainDesc.Flags));

        //BOOL fullscreenState;
        //m_SwapChain->GetFullscreenState(&fullscreenState, nullptr);
        //m_Fullscreen = fullscreenState == TRUE;

        m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

        UpdateSwapChainRenderTargetViews();
    }
}


void Window::UpdateSwapChainRenderTargetViews()
{
    auto device = Application::Get().GetDevice();

    // Get a handle to the first descriptor in the heap.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < FrameCount; ++i)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i])));
        device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(m_RTVDescriptorSize);
    }
}

void Window::SetVSync(bool vsync)
{
    m_VSync = vsync;
}

void Window::ToggleVSync()
{
    SetVSync(!IsVsync());
}

void Window::Clear(float red, float green, float blue, float alpha)
{
    FLOAT ColorRGBA[] = { red, green, blue, alpha };

    m_CommandAllocators[m_CurrentBackBufferIndex]->Reset();
    m_CommandList->Reset(m_CommandAllocators[m_CurrentBackBufferIndex].Get(), nullptr);

    // Transition back buffer to Render target. This is required to clear the back buffer.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackBufferIndex, m_RTVDescriptorSize);
    m_CommandList->ClearRenderTargetView(rtvHandle, ColorRGBA, 0, nullptr);
}

void Window::Present()
{
    if (!m_IsMinimized)
    {
        Application& app = Application::Get();

        // Transition back buffer to the present state. This is required before the back buffer can be presented.
        m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ));

        m_CommandList->Close();

        ID3D12CommandList* const ppCommandLists[] =
        {
            m_CommandList.Get()
        };
        app.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->ExecuteCommandLists(1, ppCommandLists);

        UINT syncInterval = m_VSync ? 1 : 0;
        UINT presentFlags = m_AllowTearing && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        m_SwapChain->Present(syncInterval, presentFlags);

        m_FenceValues[m_CurrentBackBufferIndex] = app.Signal(D3D12_COMMAND_LIST_TYPE_DIRECT);

        // Update the current back buffer index.
        m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // Wait until the next frame is available for rendering.
        app.WaitForFenceValue(m_FenceValues[m_CurrentBackBufferIndex]);
    }
}

void Window::Show()
{
    ::ShowWindow(m_hWindow, SW_SHOWDEFAULT);
}

void Window::Hide()
{
    ::ShowWindow(m_hWindow, SW_HIDE);
}

void Window::SetWindowTitle(const std::wstring& windowTitle)
{
    ::SetWindowTextW(m_hWindow, windowTitle.c_str());
}

void Window::SetFullscreen(bool fullscreen)
{
    if (m_Fullscreen != fullscreen)
    {
        m_Fullscreen = fullscreen;

        if (m_Fullscreen) // Switching to fullscreen.
        {
            // Store the current window dimensions so they can be restored 
            // when switching out of fullscreen state.
            ::GetWindowRect(m_hWindow, &m_WindowRect);
            
            // Set the window style to a borderless window so the client area fills
            // the entire screen.
            UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
            // Is this the same thing?
            UINT sytleDiff = windowStyle & ~WS_POPUP;

            ::SetWindowLongW(m_hWindow, GWL_STYLE, windowStyle);

            // Query the name of the nearest display device for the window.
            // This is required to set the fullscreen dimensions of the window
            // when using a multi-monitor setup.
            HMONITOR hMonitor = ::MonitorFromWindow(m_hWindow, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            ::GetMonitorInfo(hMonitor, &monitorInfo);

            // Get the settings for the primary display. These settings are used
            // to determine the correct position and size to position the window
            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);
            ::EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);

            ::SetWindowPos(m_hWindow, HWND_TOPMOST,
                devMode.dmPosition.x,
                devMode.dmPosition.y,
                devMode.dmPosition.x + devMode.dmPelsWidth,
                devMode.dmPosition.y + devMode.dmPelsHeight,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(m_hWindow, SW_MAXIMIZE);
        }
        else
        {
            // Restore all the window decorators.
            ::SetWindowLong(m_hWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);

            ::SetWindowPos(m_hWindow, HWND_NOTOPMOST,
                m_WindowRect.left,
                m_WindowRect.top,
                m_WindowRect.right - m_WindowRect.left,
                m_WindowRect.bottom - m_WindowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(m_hWindow, SW_NORMAL);
        }
    }

}

void Window::ToggleFullscreen()
{
    SetFullscreen(!GetFullscreen());
}

void Window::OnKeyPressed(KeyEventArgs& e)
{
    KeyPressed(e);
}

void Window::OnKeyReleased(KeyEventArgs& e)
{
    KeyReleased(e);
}

void Window::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (!m_IsMouseInClientArea)
    {
        m_IsMouseInClientArea = true;
        EventArgs eventArgs(*this);
        OnMouseEnter(eventArgs);
    }

    // TODO: Compute relative movement since previous position.
    
    MouseMoved(e);
}

void Window::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    MouseButtonPressed(e);
}

void Window::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    MouseButtonReleased(e);
}

void Window::OnMouseWheel(MouseWheelEventArgs& e)
{
    MouseWheel(e);
}

void Window::OnMouseLeave(EventArgs& e)
{
    m_IsMouseInClientArea = false;

    TrackMouseEvents();

    MouseLeave(e);
}

void Window::OnMouseEnter(EventArgs& e)
{
    MouseEnter(e);
}

void Window::OnResize(ResizeEventArgs& e)
{
    m_IsMinimized = e.Action == ResizeAction::Minimized;
    if ( !m_IsMinimized )
    {
        ResizeSwapChainBuffers(e.Width, e.Height);
    }

    Resize(e);
}

void Window::OnClose(WindowCloseEventArgs& e)
{
    Close(e);
}

void Window::TrackMouseEvents()
{
    TRACKMOUSEEVENT trackMouseEvent = {};
    trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
    trackMouseEvent.hwndTrack = m_hWindow;
    trackMouseEvent.dwFlags = TME_LEAVE;
    ::TrackMouseEvent(&trackMouseEvent);
}
