#include <GameFramework/GameFramework.h>

#include <Tutorial4.h>

#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <Windows.h>
#include <shellapi.h>
#include <wrl/client.h>

#include <dxgi1_3.h>
#include <dxgidebug.h>  // For IDXGIDebug1.

#include <dx12lib/Helpers.h>  // For ThrowIfFailed

using namespace dx12lib;

void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    ::DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &dxgiDebug ) );

    dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL );
    dxgiDebug->Release();
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{

#if defined( _DEBUG )
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed( ::D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) ) );
    debugInterface->EnableDebugLayer();
#endif

    int retCode = 0;

    WCHAR path[MAX_PATH];

    int     argc = 0;
    LPWSTR* argv = CommandLineToArgvW( lpCmdLine, &argc );
    if ( argv )
    {
        for ( int i = 0; i < argc; ++i )
        {
            // -wd Specify the Working Directory.
            if ( wcscmp( argv[i], L"-wd" ) == 0 )
            {
                wcscpy_s( path, argv[++i] );
                SetCurrentDirectoryW( path );
            }
        }
        LocalFree( argv );
    }

    GameFramework::Create( hInstance );
    {
        std::unique_ptr<Tutorial4> demo = std::make_unique<Tutorial4>( L"HDR", 1920, 1080, true );
        retCode                         = demo->Run();
    }
    GameFramework::Destroy();

    atexit( &ReportLiveObjects );

    return retCode;
}