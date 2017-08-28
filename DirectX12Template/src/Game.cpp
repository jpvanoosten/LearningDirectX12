#include <DirectX12TemplatePCH.h>

#include <Application.h>
#include <Window.h>

#include <Game.h>

Game::Game(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle)
    : m_WindowWidth( windowWidth )
    , m_WindowHeight( windowHeight )
    , m_WindowTitle( windowTitle )
{
    m_pWindow = Application::Get().CreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle);

    m_pWindow->Show();
}

Game::~Game()
{}