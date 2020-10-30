#include <Tutorial4.h>

#include <GameFramework/GameFramework.h>
#include <dx12lib/Device.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <shellapi.h>

using namespace dx12lib;

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{

#if defined( _DEBUG )
    Device::EnableDebugLayer();
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

    atexit( &Device::ReportLiveObjects );

    return retCode;
}