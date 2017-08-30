#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Helpers.h>
#include <Window.h>

static Application* g_pApplication = nullptr;

using WindowHandleMap = std::map<HWND, std::weak_ptr<Window>>;
WindowHandleMap gs_WindowHandles;

using namespace Microsoft::WRL;

Application::Application( HINSTANCE hInstance, int argc, const wchar_t* argv[] )
    : m_hInstance( hInstance )
    , m_FenceValue( 1 )
    , m_Quit( false )
    , m_bUseWarp( false )
    , m_bAllowTearing( false )
{
    assert(g_pApplication == nullptr && "Application instance already created.");
    g_pApplication = this;

#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif

    // Parse command line arguments.
    for (size_t i = 0; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"--warp") == 0 || wcscmp(argv[i], L"-warp") == 0)
        {
            m_bUseWarp = true;
        }
    }
    
    m_bAllowTearing = CheckTearingSupport();

    // Try to get a list of the adapters that support DX12.
    AdapterList adapters = GetAdapters(m_bUseWarp);
    if (adapters.size() < 1)
    {
        // Force using warp if no supported adapters were found.
        adapters = GetAdapters(true);
    }
    assert(adapters.size() > 0);

    // Create a device using the first adapter in the list.
    m_Device = CreateDevice( adapters[0] );

    // Create fence and event objects for GPU/CPU synchronization.
    ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
    m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_FenceEvent && "Failed to create fence event.");

    CreateCommandQueues(m_Device);
}

Application::~Application()
{
    WaitForGPU();
    ::CloseHandle(m_FenceEvent);

    g_pApplication = nullptr;
}

Application& Application::Get()
{
    assert(g_pApplication != nullptr && "No application instance created yet.");
    return *g_pApplication;
}

std::shared_ptr<Window> Application::CreateWindow(uint32_t width, uint32_t height, const std::wstring& name, bool fullscreen)
{
    std::shared_ptr<Window> pWindow = std::make_shared<Window>(width, height, name, fullscreen);
    assert(pWindow && "Failed to create window.");

    // Todo: Attach application events to the window.

    HWND hWnd = pWindow->GetWindowHandle();
    gs_WindowHandles.insert( WindowHandleMap::value_type(hWnd, pWindow));

    ::UpdateWindow(hWnd);

    return pWindow;
}

ComPtr<ID3D12CommandQueue> Application::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    ComPtr<ID3D12CommandQueue> commandQueue;
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        commandQueue = m_GraphicsCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        commandQueue = m_ComputeCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        commandQueue = m_CopyCommandQueue;
        break;
    default:
        assert(false && "Invalid command queue type.");
        break;
    }

    return commandQueue;
}


AdapterList Application::GetAdapters( bool useWarp ) const
{
    AdapterList adapters;

    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;
    if (useWarp)
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
        adapters.push_back(dxgiAdapter4);
    }
    else
    {
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter4->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                adapters.push_back(dxgiAdapter4);
            }
        }
    }

    return adapters;
}

Microsoft::WRL::ComPtr<ID3D12Device2> Application::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

    // Enable debug messages in debug mode.
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        pInfoQueue->PushStorageFilter(&NewFilter);
    }
#endif

    return d3d12Device2;
}

ComPtr<ID3D12CommandQueue> Application::CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, INT priority, D3D12_COMMAND_QUEUE_FLAGS flags, UINT nodeMask)
{
    assert(device && "Device is NULL");

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Type = type;
    commandQueueDesc.Priority = priority;
    commandQueueDesc.Flags = flags;
    commandQueueDesc.NodeMask = nodeMask;

    ComPtr<ID3D12CommandQueue> commandQueue;

    ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));

    return commandQueue;
}

void Application::CreateCommandQueues(Microsoft::WRL::ComPtr<ID3D12Device2> device)
{
    m_GraphicsCommandQueue = CreateCommandQueue(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_ComputeCommandQueue = CreateCommandQueue(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_CopyCommandQueue = CreateCommandQueue(device, D3D12_COMMAND_LIST_TYPE_COPY);
}

bool Application::CheckTearingSupport()
{
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    return allowTearing == TRUE;
}

int Application::Run( Game& game)
{
    // TODO: Init game.
    // TODO: Load game resources.

    MSG msg = {};
    while ( msg.message != WM_QUIT)
    {
        if (m_Quit)
        {
            ::PostQuitMessage(0);
            m_Quit = false;
        }

        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    return 0;
}

void Application::Stop()
{
    m_Quit = true;
}

UINT64 Application::Signal(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> commandQueue = GetCommandQueue(type);

    uint64_t fenceValue = m_FenceValue++;
    ThrowIfFailed(commandQueue->Signal(m_Fence.Get(), fenceValue));
    return fenceValue;
}

UINT64 Application::GetCompletedFenceValue() const
{
    return m_Fence->GetCompletedValue();
}

bool Application::IsFenceComplete(UINT64 fenceValue) const
{
    return GetCompletedFenceValue() >= fenceValue;
}

void Application::WaitForFenceValue(UINT64 fenceValue, std::chrono::milliseconds duration )
{
    if (!IsFenceComplete(fenceValue))
    {
        ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent));
        ::WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void Application::WaitForGPU()
{
    WaitForFenceValue(Signal(D3D12_COMMAND_LIST_TYPE_DIRECT));
    WaitForFenceValue(Signal(D3D12_COMMAND_LIST_TYPE_COMPUTE));
    WaitForFenceValue(Signal(D3D12_COMMAND_LIST_TYPE_COPY));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::shared_ptr<Window> pWindow;
    {
        WindowHandleMap::const_iterator iter = gs_WindowHandles.find(hwnd);
        if (iter != gs_WindowHandles.end())
        {
            pWindow = iter->second.lock();
            // If the weak pointer is null, just remove the window from the list
            // of known windows. This happens if the last shared_ptr to the window
            // has been released and the OS window is being destroyed.
            if (!pWindow) gs_WindowHandles.erase(iter);
        }
    }

    if (pWindow)
    {
        switch (message)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT paintStruct;
            HDC hDC;

            hDC = BeginPaint(hwnd, &paintStruct);
            EndPaint(hwnd, &paintStruct);
        }
        break;
        case WM_CLOSE:
        {
            // TODO: Dispatch an event to the window itself
            Application::Get().Stop();
        }
        break;
        case WM_DESTROY:
        {
            WindowHandleMap::iterator iter = gs_WindowHandles.find(hwnd);
            if (iter != gs_WindowHandles.end())
            {
                // Window is being destroyed. Stop tracking it.
                gs_WindowHandles.erase(iter);
            }
        }
        break;
        default:
        {
            return ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
        }
    }
    else
    {
        switch (message)
        {
        case WM_CREATE:
        {
            // 
        }
        break;
        default:
        {
            return ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
        }
    }
    
    return 0;
}
