#include <DirectX12TemplatePCH.h>
#include <DX12Game.h>

using namespace DirectX12Template;

DX12Game::DX12Game(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle)
    : m_WindowWidth( windowWidth )
    , m_WindowHeight( windowHeight )
    , m_WindowTitle( windowTitle )
{}

DX12Game::~DX12Game()
{}