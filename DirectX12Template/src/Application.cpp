#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Window.h>

static Application* g_pApplication = nullptr;

using WindowHandleMap = std::map<HWND, std::weak_ptr<Window>>;
WindowHandleMap gs_WindowHandles;

Application::Application( HINSTANCE hInstance, int argc, const wchar_t* argv[] )
    : m_hInstance( hInstance )
    , m_Quit( false )
{
    assert(g_pApplication == nullptr && "Application instance already created.");
    g_pApplication = this;
}

Application::~Application()
{
    g_pApplication = nullptr;
}

Application& Application::Get()
{
    assert(g_pApplication != nullptr && "No application instance created yet.");
    return *g_pApplication;
}

std::shared_ptr<Window> Application::CreateWindow(uint32_t width, uint32_t height, const std::wstring& name, bool fullscreen)
{
    std::shared_ptr<Window> pWindow = std::make_shared<Window>(width, height, name);
    assert(pWindow && "Failed to create window.");

    // Todo: Attach application events to the window.

    HWND hWnd = pWindow->GetWindowHandle();
    gs_WindowHandles.insert( WindowHandleMap::value_type(hWnd, pWindow));

    UpdateWindow(hWnd);

    return pWindow;
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::shared_ptr<Window> pWindow;
    {
        WindowHandleMap::const_iterator iter = gs_WindowHandles.find(hwnd);
        if (iter != gs_WindowHandles.end())
        {
            pWindow = iter->second.lock();
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
