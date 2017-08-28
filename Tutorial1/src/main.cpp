#include <Tutorial1PCH.h>
#include <DirectX12Template.h>

int CALLBACK wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc );
    Application app(hInstance, argc, const_cast<const wchar_t**>( argv ));

    Game game(1280, 720, L"Tutorial1");

    int ret = app.Run(game);

    // Free memory allocated by CommandLineToArgvW
    LocalFree(argv);

    return ret;
}