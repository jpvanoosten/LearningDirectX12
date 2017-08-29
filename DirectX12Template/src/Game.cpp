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
    m_pWindow->Show();
}

Game::~Game()
{}

