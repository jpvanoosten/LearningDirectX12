/**
 * ImGui wrapper.
 * This class is used internally by the Window class.
 */
#include "imgui.h"

#include <d3dx12.h>
#include <wrl.h>

class CommandList;
class Texture;
class RenderTarget;
class RootSignature;
class Window;


#pragma once
class GUI
{
public:
    GUI();
    virtual ~GUI();

    // Initialize the ImGui context.
    virtual bool Initialize( std::shared_ptr<Window> window );

    // Begin a new frame.
    virtual void NewFrame();
    virtual void Render( std::shared_ptr<CommandList> commandList, const RenderTarget& renderTarget );

    // Destroy the ImGui context.
    virtual void Destroy();

protected:

private:
    ImGuiContext* m_pImGuiCtx;
    std::unique_ptr<Texture> m_FontTexture;
    std::unique_ptr<RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    std::shared_ptr<Window> m_Window;
};