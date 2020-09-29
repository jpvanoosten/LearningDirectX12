#include <GameFrameworkPCH.h>
#include <Application.h>

static Application* gs_pSingelton = nullptr;

Application& Application::Create( HINSTANCE hInst )
{
    if ( !gs_pSingelton )
    {
        gs_pSingelton = new Application( hInst );
    }

    return *gs_pSingelton;
}
