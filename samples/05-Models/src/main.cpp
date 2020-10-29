#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/SwapChain.h>

using namespace dx12lib;

void OnUpdate( UpdateEventArgs& e );
void OnKeyPressed( KeyEventArgs& e );
void OnWindowResized( ResizeEventArgs& e );
void OnWindowClose( WindowCloseEventArgs& e );

std::shared_ptr<Window>    pGameWindow = nullptr;
std::shared_ptr<Device>    pDevice     = nullptr;
std::shared_ptr<SwapChain> pSwapChain  = nullptr;

Logger logger;

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{
#if defined( _DEBUG )
    // Always enable the Debug layer before doing anything with DX12.
    Device::EnableDebugLayer();
#endif

    int retCode = 0;

    auto& gf = GameFramework::Create( hInstance );
    {
        // Create a logger for logging messages.
        logger = gf.CreateLogger( "ClearScreen" );

        // Create a GPU device using the default adapter selection.
        pDevice = Device::Create();

        auto description = pDevice->GetDescription();
        logger->info( L"Device Created: {}", description );

        // Create a window:
        pGameWindow = gf.CreateWindow( L"Clear Screen", 1920, 1080 );

        // Create a swap chain for the window
        pSwapChain = pDevice->CreateSwapChain( pGameWindow->GetWindowHandle() );
        pSwapChain->SetVSync( false );

        // Register events.
        pGameWindow->KeyPressed += &OnKeyPressed;
        pGameWindow->Resize += &OnWindowResized;
        pGameWindow->Update += &OnUpdate;
        pGameWindow->Close += &OnWindowClose;

        pGameWindow->Show();

        retCode = GameFramework::Get().Run();

        // Release globals.
        pSwapChain.reset();
        pGameWindow.reset();
        pDevice.reset();
    }
    // Destroy game framework resource.
    GameFramework::Destroy();

    atexit( &Device::ReportLiveObjects );

    return retCode;
}

void OnUpdate( UpdateEventArgs& e )
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.DeltaTime;
    ++frameCount;

    if ( totalTime > 1.0 )
    {
        auto fps   = frameCount / totalTime;
        frameCount = 0;
        totalTime  = 0.0;

        logger->info( "FPS: {:.7}", fps );

        wchar_t buffer[256];
        ::swprintf_s( buffer, L"Clear Screen [FPS: %f]", fps );
        pGameWindow->SetWindowTitle( buffer );
    }

    auto& commandQueue = pDevice->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto  commandList  = commandQueue.GetCommandList();

    auto& renderTarget = pSwapChain->GetRenderTarget();

    const FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
    commandList->ClearTexture( renderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );

    commandQueue.ExecuteCommandList( commandList );

    pSwapChain->Present();
}

void OnKeyPressed( KeyEventArgs& e )
{
    logger->info( L"KeyPressed: {}", (wchar_t)e.Char );

    switch ( e.Key )
    {
    case KeyCode::V:
        pSwapChain->ToggleVSync();
        break;
    case KeyCode::Escape:
        // Stop the application if the Escape key is pressed.
        GameFramework::Get().Stop();
        break;
    case KeyCode::Enter:
        if ( e.Alt )
        {
            [[fallthrough]];
        case KeyCode::F11:
            pGameWindow->ToggleFullscreen();
            break;
        }
    }
}

void OnWindowResized( ResizeEventArgs& e )
{
    logger->info( "Window Resize: {}, {}", e.Width, e.Height );
    pSwapChain->Resize( e.Width, e.Height );
}

void OnWindowClose( WindowCloseEventArgs& e )
{
    // Stop the application if the window is closed.
    GameFramework::Get().Stop();
}
