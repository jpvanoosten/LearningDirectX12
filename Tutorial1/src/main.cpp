#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>

// Helper functions
#include <Helpers.h>

// The number of swap chain back buffers.
const uint8_t g_NumFrames = 2;
// The current back buffer index to render to.
uint8_t g_CurrentBackBufferIndex;

// Use WARP adapter
bool g_UseWarp = false;

// DirectX12 Objects.
ComPtr<ID3D12Device2> g_D3D12Device;
ComPtr<ID3D12Fence> g_D3D12Fence;
ComPtr<ID3D12CommandQueue> g_D3D12CommandQueue;
ComPtr<ID3D12GraphicsCommandList> g_D3D12CommandList;
ComPtr<ID3D12CommandAllocator> g_D3D12CommandAllocator[g_NumFrames];

// DXGI Objects
ComPtr<IDXGISwapChain4> g_DXGISwapChain;

// The next value to use to signal the command queue.
uint64_t g_FenceValue = 1;
// The per-frame fence values. Used to synchronize the swap chain back buffers.
uint64_t g_FrameFenceValues[g_NumFrames] = {};

// Windows event handle. Used for waiting for the fence value to be reached.
HANDLE g_FenceEvent;

// Application instance handle
HINSTANCE g_hInstance;

// Window class name. Used for registering / creating the window.
const wchar_t* g_WindowClassName = L"DX12WindowClass";
// Window title will be displayed on the title bar of the window.
const wchar_t* g_WindowTitle = L"Learning DirectX 12";

// Window handle.
HWND g_hWindow;

uint32_t g_ClientWidth = 1280;
uint32_t g_ClientHeight = 720;

// Window registration and window creation.
void RegisterWindowClass();
HWND CreateWindow( const wchar_t* windowTitle, uint32_t width, uint32_t height);

// Window callback function.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Utility functions.
void ParseCommandLineArguments();

// DirectX 12 Initialization functions

void EnableDebugLayer();
ComPtr<IDXGIAdapter4> GetAdapter( bool useWarp );

ComPtr<ID3D12Device2> CreateDevice( ComPtr<IDXGIAdapter4> adapter );

ComPtr<ID3D12CommandQueue> CreateCommandQueue( ComPtr<ID3D12Device2> device, 
    D3D12_COMMAND_LIST_TYPE type );

// Check for tearing support.
bool CheckTearingSupport();

ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, 
    ComPtr<ID3D12CommandQueue> commandQueue, 
    uint32_t width, uint32_t height, uint32_t bufferCount );

ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);

HANDLE CreateEventHandle();

ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12Device2> device,
    D3D12_COMMAND_LIST_TYPE type);

ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device);

// Synchronization functions

// Signal the command queue.
// Returns the fence value to wait for.
uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue );

// Waits for a fence value to be reached before continuing.
void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, 
    std::chrono::milliseconds duration = std::chrono::milliseconds::max());

// Make sure the command queue has completed all commands before continuing.
void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue, HANDLE fenceEvent );


// Event callback functions.
void Update();
void Render();
void Resize(uint32_t width, uint32_t height);

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    ParseCommandLineArguments();
    EnableDebugLayer();

    RegisterWindowClass();
    g_hWindow = CreateWindow( g_WindowTitle, g_ClientWidth, g_ClientHeight);

    ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(g_UseWarp);
    g_D3D12Device = CreateDevice(dxgiAdapter4);
    g_D3D12CommandQueue = CreateCommandQueue(g_D3D12Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    g_DXGISwapChain = CreateSwapChain(g_hWindow, g_D3D12CommandQueue,
        g_ClientWidth, g_ClientHeight, g_NumFrames);

    g_D3D12Fence = CreateFence(g_D3D12Device);
    g_FenceEvent = CreateEventHandle();

    ::ShowWindow(g_hWindow, SW_SHOW);


    ::CloseHandle(g_FenceEvent);
    
    int ret = 0;
    return ret;
}

void ParseCommandLineArguments()
{
    int argc;
    wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    for (size_t i = 0; i < argc; ++i)
    {
        if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
        {
            g_ClientWidth = ::wcstol(argv[++i], nullptr, 10);
        }
        if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
        {
            g_ClientHeight = ::wcstol(argv[++i], nullptr, 10);
        }
        if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
        {
            g_UseWarp = true;
        }
    }

    // Free memory allocated by CommandLineToArgvW
    LocalFree(argv);
}

void EnableDebugLayer()
{
#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif

}

void RegisterWindowClass()
{
    // Register a window class for creating our render window with.
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = g_hInstance;
    windowClass.hIcon = ::LoadIcon(g_hInstance, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = g_WindowClassName;
    windowClass.hIconSm = ::LoadIcon(g_hInstance, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));

    static HRESULT hr = ::RegisterClassExW(&windowClass);
    assert(SUCCEEDED(hr));
}

HWND CreateWindow(const wchar_t* windowTitle, uint32_t width, uint32_t height)
{
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    HWND hWnd = ::CreateWindowExW(
        NULL,
        g_WindowClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        g_hInstance,
        nullptr
    );

    assert(hWnd && "Failed to create window");

    return hWnd;
}

ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;
    SIZE_T maxDedicatedVideoMemory = 0;

    if (useWarp)
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            ComPtr<IDXGIAdapter4> tmpAdapter;
            ThrowIfFailed(dxgiAdapter1.As(&tmpAdapter));
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            tmpAdapter->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter4.Get(), 
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && 
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory )
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                dxgiAdapter4 = tmpAdapter;
            }
        }
    }

    return dxgiAdapter4;
}

ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
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

ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type )
{
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type =     type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags =    D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

    return d3d12CommandQueue;
}

bool CheckTearingSupport()
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

ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, 
    ComPtr<ID3D12CommandQueue> commandQueue, 
    uint32_t width, uint32_t height, uint32_t bufferCount )
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

    //m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    //UpdateSwapChainRenderTargetViews();

    return dxgiSwapChain4;
}

ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device)
{
    ComPtr<ID3D12Fence> fence;

    // Create fence and event objects for GPU/CPU synchronization.
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

HANDLE CreateEventHandle()
{
    HANDLE fenceEvent;
    
    fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent && "Failed to create fence event.");

    return fenceEvent;
}

uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue)
{
    uint64_t fenceValueForSignal = fenceValue++;
    ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent,
    std::chrono::milliseconds duration)
{
    if (fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
        ::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue, HANDLE fenceEvent )
{
    uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
    WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}


void Update()
{

}

void Render()
{

}

void Resize(uint32_t width, uint32_t height)
{

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        Update();
        Render();

    }
    break;
    case WM_SIZE:
    {
        RECT clientRect = {};
        ::GetClientRect(hwnd, &clientRect);

        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        Resize(width, height);
    }
    break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            ::PostQuitMessage(0);
            break;
        }
        
        break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
        // Currently does nothing.
        
        break;
    default:
        return ::DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}