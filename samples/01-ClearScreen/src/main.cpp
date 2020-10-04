#include <Application.h>
#include <Window.h>

#include <spdlog/spdlog.h>

void OnUpdate( UpdateEventArgs& e );
void OnKeyPressed( KeyEventArgs& e );
void OnKeyReleased( KeyEventArgs& e );
void OnWindowResized( ResizeEventArgs& e );
void OnWindowClose( WindowCloseEventArgs& e );

std::shared_ptr<Window> pGameWindow = nullptr;

int CALLBACK wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       PWSTR lpCmdLine, int nCmdShow )
{
    int retCode = 0;

    auto& app = Application::Create( hInstance );
    {
        // Create a window:
        pGameWindow = app.CreateWindow( L"Clear Screen", 1920, 1080 );

        // Register events.
        pGameWindow->KeyPressed += &OnKeyPressed;
        pGameWindow->KeyReleased += &OnKeyReleased;
        pGameWindow->Close += &OnWindowClose;
        pGameWindow->Resize += &OnWindowResized;
        pGameWindow->Update += &OnUpdate;

        pGameWindow->Show();

        retCode = Application::Get().Run();

        pGameWindow.reset();
    }
    Application::Destroy();

    return retCode;
}

void OnUpdate( UpdateEventArgs& e )
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.DeltaTime;
    frameCount++;

    if ( totalTime > 1.0 )
    {
        auto fps       = frameCount / totalTime;
        frameCount = 0;
        totalTime -= 1.0;

        spdlog::info( "FPS: {:.7}", fps );
    }
}

void OnKeyPressed( KeyEventArgs& e )
{
    spdlog::info( L"KeyPressed: {}", (wchar_t)e.Char );

    switch ( e.Key )
    {
    case KeyCode::Escape:
        // Stop the application if the Escape key is pressed.
        Application::Get().Stop();
        break;
    case KeyCode::Enter:
        if ( e.Alt )
        {
    case KeyCode::F11:
            pGameWindow->ToggleFullscreen();
            break;
        }
    }
}

void OnKeyReleased( KeyEventArgs& e )
{
    spdlog::info( L"KeyReleased: {}", (wchar_t)e.Char );
}

void OnWindowResized( ResizeEventArgs& e )
{
    spdlog::info( "Window Resize: {}, {}", e.Width, e.Height );
}

void OnWindowClose( WindowCloseEventArgs& e )
{
    // Stop the application if the window is closed.
    Application::Get().Stop();
}
