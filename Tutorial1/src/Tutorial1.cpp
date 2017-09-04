#include <Tutorial1PCH.h>
#include <Tutorial1.h>

Tutorial1::Tutorial1(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle, bool fullscreen, bool vsync)
{
    // Connect application events:
    Application& app = Application::Get();

    app.Init += boost::bind(&Tutorial1::OnInit, this, _1);
    app.LoadResources += boost::bind(&Tutorial1::OnLoadResources, this, _1);
    app.Update += boost::bind(&Tutorial1::OnUpdate, this, _1);
    app.Render += boost::bind(&Tutorial1::OnRender, this, _1);

    m_pWindow = app.CreateWindow(windowWidth, windowHeight, windowTitle, fullscreen, vsync);

    // Connect window events:
    m_pWindow->KeyPressed += boost::bind(&Tutorial1::OnKeyPressed, this, _1);
    m_pWindow->KeyReleased += boost::bind(&Tutorial1::OnKeyReleased, this, _1);

    m_pWindow->Close += boost::bind(&Tutorial1::OnWindowClose, this, _1);

    // Show the window.
    m_pWindow->Show();

}

void Tutorial1::OnInit(EventArgs& e)
{

}

void Tutorial1::OnLoadResources(EventArgs& e)
{

}

void Tutorial1::OnStart(EventArgs& e)
{

}

void Tutorial1::OnUpdate(UpdateEventArgs& e)
{

}

void Tutorial1::OnRender(RenderEventArgs& e)
{
    // Clear the screen to approximately "Cornflower blue".
    m_pWindow->Clear(0.4f, 0.58f, 0.93f);

    // TODO: Render stuff.

    // Present the window to display the contents.
    m_pWindow->Present();
}

void Tutorial1::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Stop();
        break;
    case KeyCode::Enter:
        if (e.Alt)
        {
    case KeyCode::F11:
        m_pWindow->ToggleFullscreen();
        }
        break;
    case KeyCode::V:
        m_pWindow->ToggleVSync();
        break;
    }
}

void Tutorial1::OnKeyReleased(KeyEventArgs& e)
{

}

void Tutorial1::OnWindowClose(WindowCloseEventArgs& e)
{
    // If the primary window is closing, just exit the application.
    Application::Get().Stop();
}
