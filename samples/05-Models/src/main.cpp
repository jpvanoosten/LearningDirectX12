#include <Tutorial5.h>

#include <dx12lib/Device.h>

#include <shellapi.h> // For CommandLineToArgvW

using namespace dx12lib;

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{
#if defined( _DEBUG )
    // Always enable the Debug layer before doing anything with DX12.
    Device::EnableDebugLayer();
#endif

    // Set the working directory
    WCHAR path[MAX_PATH];
    int     argc = 0;
    LPWSTR* argv = ::CommandLineToArgvW( lpCmdLine, &argc );
    if ( argv )
    {
        for ( int i = 0; i < argc; ++i )
        {
            // -wd Specify the Working Directory.
            if ( ::wcscmp( argv[i], L"-wd" ) == 0 )
            {
                ::wcscpy_s( path, argv[++i] );
                ::SetCurrentDirectoryW( path );
            }
        }
        ::LocalFree( argv );
    }

    int retCode = 0;

    GameFramework::Create( hInstance );
    {
        auto demo = std::make_unique<Tutorial5>( L"Models", 1920, 1080 );
        retCode = demo->Run();
    }
    // Destroy game framework resource.
    GameFramework::Destroy();

    ::atexit( &Device::ReportLiveObjects );

    return retCode;
}