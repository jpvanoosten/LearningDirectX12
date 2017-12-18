#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Application.h>
#include <Tutorial2.h>


int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    Application::Create(hInstance);

    std::shared_ptr<Tutorial2> demo = std::make_shared<Tutorial2>(L"Learning DirectX 12 - Lesson 2", 1280, 720);

    int retCode = Application::Get().Run(demo);

    return retCode;
}