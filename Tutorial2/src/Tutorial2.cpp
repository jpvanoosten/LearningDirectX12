#include <Tutorial2.h>

#include <Application.h>
#include <Window.h>

Tutorial2::Tutorial2( const std::wstring& name, int width, int height, bool vSync )
    : super(name, width, height, vSync)
{
}

bool Tutorial2::LoadContent()
{
    return true;
}

void Tutorial2::UnloadContent()
{

}

void Tutorial2::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;
    
    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        char buffer[512];
        sprintf_s(buffer, "FPS: %f\n", frameCount, totalTime, fps);
        OutputDebugString(buffer);

        frameCount = 0;
        totalTime = 0.0;
    }
}

void Tutorial2::OnRender(RenderEventArgs& e)
{

}

void Tutorial2::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
    break;
    }
}
