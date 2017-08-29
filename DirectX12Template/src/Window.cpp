#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Helpers.h>

#include <Window.h>

// Import namespaces.
using namespace Microsoft::WRL;

// Constants
constexpr auto WINDOW_CLASS_NAME = L"DX12WindowClass";

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Defined in the Application class.

Window::Window(uint32_t width, uint32_t height, const std::wstring& name, bool fullscreen )
    : m_Width( width )
    , m_Height( height )
    , m_Fullscreen( fullscreen )
    , m_Name( name )
    , m_FenceValues{}
    , m_CurrentFrameIndex( 0 )
{
    // Create a windows event object that will be used for GPU -> CPU syncronization.
    m_FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

    // Create the GPU fence object to signal the GPU command queue.
    ComPtr<ID3D12Device2> device = Application::Get().GetDevice();
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
    // Increment the fence value for the current thread. This is the next value
    // to use to signal the command queue.
    m_FenceValues[m_CurrentFrameIndex]++;

    CreateWindow();
    CreateSwapChain();
}

Window::~Window()
{
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
    WNDCLASSEXW windowClass = GetWindowClassInfo(hInstance);

    // Store the result in a local static to ensure this function is called only once.
    static HRESULT hr = ::RegisterClassExW(&windowClass);

    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

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

    ::SetWindowTextW(m_hWindow, m_Name.c_str());
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
    ComPtr<ID3D12CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    // Make sure all GPU commands are finished before (re) creating the swap chain for this window.
    Application::Get().WaitForGPU();

    // TODO: Create swap chain and render targets for swap chain.
    
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