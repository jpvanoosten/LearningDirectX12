#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Window.h>

#include <Game.h>

Game::Game(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle, bool fullscreen )
    : m_WindowWidth( windowWidth )
    , m_WindowHeight( windowHeight )
    , m_Fullscreen( fullscreen )
    , m_WindowTitle( windowTitle )
{
    m_pWindow = Application::Get().CreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle, m_Fullscreen);

    // Connect events:
    m_pWindow->KeyPressed += boost::bind(&Game::OnKeyPressed, this, _1);
    m_pWindow->KeyReleased += boost::bind(&Game::OnKeyReleased, this, _1);

    m_pWindow->Close += boost::bind(&Game::OnWindowClose, this, _1);

    m_pWindow->Show();
}

Game::~Game()
{}


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
