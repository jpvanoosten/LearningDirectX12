#include <Application.h>
#include <Window.h>

#include <spdlog/spdlog.h>

void OnKeyPressed( KeyEventArgs& e );
void OnKeyReleased( KeyEventArgs& e );
void OnWindowClose( WindowCloseEventArgs& e );

std::shared_ptr<Window> pGameWindow = nullptr;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR lpCmdLine, int nCmdShow)
{
    int retCode = 0;
    auto& app = Application::Create(hInstance);
    {
        // Create a window:
        pGameWindow = app.CreateWindow( L"Clear Screen", 1920, 1080 );

        // Register events.
        pGameWindow->KeyPressed += &OnKeyPressed;
        pGameWindow->KeyReleased += &OnKeyReleased;
        pGameWindow->Close += &OnWindowClose;

        pGameWindow->Show();

        retCode = Application::Get().Run();

        pGameWindow->Hide();

        pGameWindow.reset();
    }
    Application::Destroy();

    return retCode;
}

void OnKeyPressed(KeyEventArgs& e) {
    spdlog::info( L"KeyPressed: {}", (wchar_t)e.Char );

    switch ( e.Key )
    {
    case KeyCode::Escape:
        // Stop the application if the Escape key is pressed.
        Application::Get().Stop();
        break;
    }
}

void OnKeyReleased( KeyEventArgs& e )
{
    spdlog::info( L"KeyReleased: {}", (wchar_t)e.Char );

}
void OnWindowClose(WindowCloseEventArgs& e) 
{
    // Stop the application if the window is closed.
    Application::Get().Stop();
}
