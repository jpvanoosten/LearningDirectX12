#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Window.h>

#include <Game.h>

Game::Game(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle, bool fullscreen, bool vsync )
{
    // Connect application events:
    Application& app = Application::Get();

    app.Init += boost::bind(&Game::OnInit, this, _1);
    app.LoadResources += boost::bind(&Game::OnLoadResources, this, _1);
    app.Update += boost::bind(&Game::OnUpdate, this, _1);
    app.Render += boost::bind(&Game::OnRender, this, _1);

    m_pWindow = app.CreateWindow(windowWidth, windowHeight, windowTitle, fullscreen, vsync);

    // Connect window events:
    m_pWindow->KeyPressed += boost::bind(&Game::OnKeyPressed, this, _1);
    m_pWindow->KeyReleased += boost::bind(&Game::OnKeyReleased, this, _1);

    m_pWindow->Close += boost::bind(&Game::OnWindowClose, this, _1);

    // Show the window.
    m_pWindow->Show();
}

Game::~Game()
{}


uint32_t Game::GetWindowWidth() const
{
    assert(m_pWindow && "Invalid window handle.");
    return m_pWindow->GetWidth();
}
uint32_t Game::GetWindowHeight() const
{
    assert(m_pWindow && "Invalid window handle.");
    return m_pWindow->GetHeight();
}

const std::wstring& Game::GetWindowTitle() const
{
    assert(m_pWindow && "Invalid window handle.");
    return m_pWindow->GetWindowTitle();
}

void Game::OnInit(EventArgs& e)
{

}

void Game::OnLoadResources(EventArgs& e)
{

}

void Game::OnStart(EventArgs& e)
{

}

void Game::OnUpdate(UpdateEventArgs& e)
{

}

void Game::OnRender(RenderEventArgs& e)
{
    // TODO: Render stuff.
    m_pWindow->Present();
}

void Game::OnKeyPressed(KeyEventArgs& e)
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

void Game::OnKeyReleased(KeyEventArgs& e)
{

}

void Game::OnWindowClose(WindowCloseEventArgs& e)
{
    // If the primary window is closing, just exit the application.
    Application::Get().Stop();
}
