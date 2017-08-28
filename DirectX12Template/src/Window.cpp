#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Helpers.h>

#include <Window.h>


// Constants
constexpr auto WINDOW_CLASS_NAME = L"DX12WindowClass";

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // Defined in the Application class.

Window::Window(uint32_t width, uint32_t height, const std::wstring& name)
    : m_Width( width )
    , m_Height( height )
    , m_Name( name )
{
    CreateWindow();
}

void Window::CreateWindow()
{
    HINSTANCE hInstance = Application::Get().GetInstanceHandle();
    WNDCLASSEXW windowClass = GetWindowClassInfo(hInstance);

    // Store the result in a local static to ensure this function is called only once.
    static HRESULT hr = RegisterClassExW(&windowClass);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    m_hWindow = CreateWindowExW(
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

    SetWindowTextW(m_hWindow, m_Name.c_str());
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


void Window::Show()
{
    ::ShowWindow(m_hWindow, SW_SHOWDEFAULT);
}

void Window::Hide()
{
    ::ShowWindow(m_hWindow, SW_HIDE);
}
