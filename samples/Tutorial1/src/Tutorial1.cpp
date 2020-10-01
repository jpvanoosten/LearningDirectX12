#include <Tutorial1PCH.h>
#include <Tutorial1.h>

Tutorial1::Tutorial1(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle, bool fullscreen, bool vsync)
{
    // Connect application events:
    Application& app = Application::Get();

    m_pWindow = app.CreateWindow(windowWidth, windowHeight, windowTitle, fullscreen, vsync);

    // Connect window events:
    m_pWindow->Update += boost::bind(&Tutorial1::OnUpdate, this, _1);
    m_pWindow->Render += boost::bind(&Tutorial1::OnRender, this, _1);
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
    Window& window = dynamic_cast<Window&>(e.Caller);

    // Clear the window contents to "Cornflower blue".
    window.Clear(0.4f, 0.58f, 0.93f);

    // TODO: Render stuff.

    // Present the window to display the contents.
    window.Present();
}

void Tutorial1::OnKeyPressed(KeyEventArgs& e)
{
    Window& window = dynamic_cast<Window&>(e.Caller);

    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Stop();
        break;
    case KeyCode::Enter:
        if (e.Alt)
        {
    case KeyCode::F11:
        window.ToggleFullscreen();
        }
        break;
    case KeyCode::V:
        window.ToggleVSync();
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
