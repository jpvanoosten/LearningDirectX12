#include <Tutorial5.h>

#include <dx12lib/Device.h>

using namespace dx12lib;

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{
#if defined( _DEBUG )
    // Always enable the Debug layer before doing anything with DX12.
    Device::EnableDebugLayer();
#endif

    int retCode = 0;

    GameFramework::Create( hInstance );
    {
        auto demo = std::make_unique<Tutorial5>( L"Models", 1920, 1080 );
        retCode = demo->Run();
    }
    // Destroy game framework resource.
    GameFramework::Destroy();

    atexit( &Device::ReportLiveObjects );

    return retCode;
}