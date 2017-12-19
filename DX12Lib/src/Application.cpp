#include <DX12LibPCH.h>
#include <Application.h>
#include "..\resource.h"

#include <Game.h>
#include <Window.h>

constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12RenderWindowClass";

using WindowPtr = std::shared_ptr<Window>;
using WindowMap = std::map< HWND, WindowPtr >;
using WindowNameMap = std::map< std::wstring, WindowPtr >;

static Application* gs_pSingelton = nullptr;
static WindowMap gs_Windows;
static WindowNameMap gs_WindowByName;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// A wrapper struct to allow shared pointers for the window class.
struct MakeWindow : public Window 
{
    MakeWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
        : Window(hWnd, windowName, clientWidth, clientHeight, vSync)
    {}
};

Application::Application(HINSTANCE hInst)
    : m_hInstance(hInst)
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif

    WNDCLASSEXW wndClass = { 0 };

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance = m_hInstance;
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    wndClass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));

    if (!RegisterClassExW(&wndClass))
    {
        MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
    }

    m_dxgiAdapter = GetAdapter(false);
    if ( m_dxgiAdapter )
    {
        m_d3d12Device = CreateDevice(m_dxgiAdapter);
    }
    if (m_d3d12Device)
    {

    }
}

void Application::Create(HINSTANCE hInst)
{
    if (!gs_pSingelton)
    {
        gs_pSingelton = new Application(hInst);
    }
}

Application& Application::Get()
{
    assert(gs_pSingelton);
    return *gs_pSingelton;
}

void Application::Destroy()
{
    if (gs_pSingelton)
    {
        gs_Windows.clear();
        gs_WindowByName.clear();

        Application* pThis = gs_pSingelton;
        gs_pSingelton = nullptr;
        delete pThis;
    }
}

Application::~Application()
{
    Destroy();
}

Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::GetAdapter(bool bUseWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (bUseWarp)
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        SIZE_T maxDedicatedVideoMemory = 0;
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }

    return dxgiAdapter4;
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

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif

    return d3d12Device2;
}

std::shared_ptr<Window> Application::CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync )
{
    // First check if a window with the given name already exists.
    WindowNameMap::iterator windowIter = gs_WindowByName.find(windowName);
    if (windowIter != gs_WindowByName.end())
    {
        return windowIter->second;
    }

    RECT windowRect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(WINDOW_CLASS_NAME, windowName.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, m_hInstance, nullptr);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    WindowPtr pWindow = std::make_shared<MakeWindow>(hWnd, windowName, clientWidth, clientHeight, vSync);

    gs_Windows.insert(WindowMap::value_type(hWnd, pWindow));
    gs_WindowByName.insert(WindowNameMap::value_type(windowName, pWindow));

    return pWindow;
}

void Application::DestroyWindow(std::shared_ptr<Window> window)
{
    if (window) window->Destroy();
}

void Application::DestroyWindow(const std::wstring& windowName)
{
    WindowPtr pWindow = GetWindowByName(windowName);
    if ( pWindow )
    {
        DestroyWindow(pWindow);
    }
}

std::shared_ptr<Window> Application::GetWindowByName(const std::wstring& windowName)
{
    std::shared_ptr<Window> window;
    WindowNameMap::iterator iter = gs_WindowByName.find(windowName);
    if (iter != gs_WindowByName.end())
    {
        window = iter->second;
    }

    return window;
}


int Application::Run(std::shared_ptr<Game> pGame)
{
    if (!pGame->Initialize()) return 1;
    if (!pGame->LoadContent()) return 2;

    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pGame->UnloadContent();
    pGame->Destroy();

    return static_cast<int>(msg.wParam);
}

void Application::Quit(int exitCode)
{
    PostQuitMessage(exitCode);
}

Microsoft::WRL::ComPtr<ID3D12Device2> Application::GetDevice() const
{
    return m_d3d12Device;
}

// Remove a window from our window lists.
static void RemoveWindow(HWND hWnd)
{
    WindowMap::iterator windowIter = gs_Windows.find(hWnd);
    if (windowIter != gs_Windows.end())
    {
        WindowPtr pWindow = windowIter->second;
        gs_WindowByName.erase(pWindow->get_WindowName());
        gs_Windows.erase(windowIter);
    }
}

// Convert the message ID into a MouseButton ID
MouseButtonEventArgs::MouseButton DecodeMouseButton(UINT messageID)
{
    MouseButtonEventArgs::MouseButton mouseButton = MouseButtonEventArgs::None;
    switch (messageID)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Left;
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Right;
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Middel;
    }
    break;
    }

    return mouseButton;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WindowPtr pWindow;
    {
        WindowMap::iterator iter = gs_Windows.find(hwnd);
        if (iter != gs_Windows.end())
        {
            pWindow = iter->second;
        }
    }

    if ( pWindow )
    {
        switch (message)
        {
        case WM_PAINT:
        {
            // Delta time will be filled in by the Window.
            UpdateEventArgs updateEventArgs(0.0f, 0.0f);
            pWindow->OnUpdate(updateEventArgs);
            RenderEventArgs renderEventArgs(0.0f, 0.0f);
            // Delta time will be filled in by the Window.
            pWindow->OnRender(renderEventArgs);
        }
        break;
        case WM_KEYDOWN:
        {
            MSG charMsg;
            // Get the Unicode character (UTF-16)
            unsigned int c = 0;
            // For printable characters, the next message will be WM_CHAR.
            // This message contains the character code we need to send the KeyPressed event.
            // Inspired by the SDL 1.2 implementation.
            if (PeekMessage(&charMsg, hwnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
            {
                GetMessage(&charMsg, hwnd, 0, 0);
                c = charMsg.wParam;
            }
            bool shift = GetAsyncKeyState(VK_SHIFT) > 0;
            bool control = GetAsyncKeyState(VK_CONTROL) > 0;
            bool alt = GetAsyncKeyState(VK_MENU) > 0;
            KeyCode::Key key = (KeyCode::Key)wParam;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;
            KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Pressed, shift, control, alt);
            pWindow->OnKeyPressed(keyEventArgs);
        }
        break;
        case WM_KEYUP:
        {
            bool shift = GetAsyncKeyState(VK_SHIFT) > 0;
            bool control = GetAsyncKeyState(VK_CONTROL) > 0;
            bool alt = GetAsyncKeyState(VK_MENU) > 0;
            KeyCode::Key key = (KeyCode::Key)wParam;
            unsigned int c = 0;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

            // Determine which key was released by converting the key code and the scan code
            // to a printable character (if possible).
            // Inspired by the SDL 1.2 implementation.
            unsigned char keyboardState[256];
            GetKeyboardState(keyboardState);
            wchar_t translatedCharacters[4];
            if (int result = ToUnicodeEx(wParam, scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
            {
                c = translatedCharacters[0];
            }

            KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Released, shift, control, alt);
            pWindow->OnKeyReleased(keyEventArgs);
        }
        break;
        case WM_MOUSEMOVE:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseMotionEventArgs mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
            pWindow->OnMouseMoved(mouseMotionEventArgs);
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Pressed, lButton, mButton, rButton, control, shift, x, y);
            pWindow->OnMouseButtonPressed(mouseButtonEventArgs);
        }
        break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Released, lButton, mButton, rButton, control, shift, x, y);
            pWindow->OnMouseButtonReleased(mouseButtonEventArgs);
        }
        break;
        case WM_MOUSEWHEEL:
        {
            // The distance the mouse wheel is rotated.
            // A positive value indicates the wheel was rotated to the right.
            // A negative value indicates the wheel was rotated to the left.
            float zDelta = ((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA;
            short keyStates = (short)LOWORD(wParam);

            bool lButton = (keyStates & MK_LBUTTON) != 0;
            bool rButton = (keyStates & MK_RBUTTON) != 0;
            bool mButton = (keyStates & MK_MBUTTON) != 0;
            bool shift = (keyStates & MK_SHIFT) != 0;
            bool control = (keyStates & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            // Convert the screen coordinates to client coordinates.
            POINT clientToScreenPoint;
            clientToScreenPoint.x = x;
            clientToScreenPoint.y = y;
            ScreenToClient(hwnd, &clientToScreenPoint);

            MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, (int)clientToScreenPoint.x, (int)clientToScreenPoint.y);
            pWindow->OnMouseWheel(mouseWheelEventArgs);
        }
        break;
        case WM_SIZE:
        {
            int width = ((int)(short)LOWORD(lParam));
            int height = ((int)(short)HIWORD(lParam));

            ResizeEventArgs resizeEventArgs(width, height);
            pWindow->OnResize(resizeEventArgs);
        }
        break;
        case WM_DESTROY:
        {
            // If a window is being destroyed, remove it from the 
            // window maps.
            RemoveWindow(hwnd);

            if (gs_Windows.empty())
            {
                // If there are no more windows, quit the application.
                PostQuitMessage(0);
            }
        }
        break;
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    else
    {
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}