#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file Tutorial3.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Tutorial 3 class.
 */

#include <Camera.h>
#include <Light.h>

#include <GameFramework/GameFramework.h>
#include <GameFramework/Events.h>

#include <dx12lib/RenderTarget.h>

#include <cstdint>  // For uint32_t
#include <memory>   // For std::unique_ptr and std::smart_ptr
#include <string>   // For std::wstring
#include <vector>   // For std::vector

namespace dx12lib
{

class CommandList;
class Device;
class GUI;
class Mesh;
class RootSignature;
class PipelineStateObject;
class ShaderResourceView;
class SwapChain;
class Texture;
}  // namespace dx12lib

class Window;  // From GameFramework

class Tutorial3
{
public:
    Tutorial3( const std::wstring& name, uint32_t width, uint32_t height, bool vSync = false );
    virtual ~Tutorial3();

    /**
     * Start the game loop and return the error code.
     */
    uint32_t Run();

    /**
     *  Load content required for the demo.
     */
    bool LoadContent();

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    void UnloadContent();

protected:
    /**
     *  Update the game logic.
     */
    void OnUpdate( UpdateEventArgs& e );
    void OnRender();
    void OnGUI(const std::shared_ptr<dx12lib::CommandList>& commandList, const dx12lib::RenderTarget& renderTarget );

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    void OnKeyPressed( KeyEventArgs& e );

    /**
     * Invoked when a key on the keyboard is released.
     */
    virtual void OnKeyReleased( KeyEventArgs& e );

    /**
     * Invoked when the mouse is moved over the registered window.
     */
    virtual void OnMouseMoved( MouseMotionEventArgs& e );

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    void OnMouseWheel( MouseWheelEventArgs& e );

    /**
     * Invoked when the window is resized.
     */
    void OnResize( ResizeEventArgs& e );

private:
    std::shared_ptr<Window> m_Window;  // Render window (from GameFramework)

    // DX12 Device.
    std::shared_ptr<dx12lib::Device>    m_Device;
    std::shared_ptr<dx12lib::SwapChain> m_SwapChain;
    std::shared_ptr<dx12lib::GUI>       m_GUI;

    // Some geometry to render.
    std::unique_ptr<dx12lib::Mesh> m_CubeMesh;
    std::unique_ptr<dx12lib::Mesh> m_SphereMesh;
    std::unique_ptr<dx12lib::Mesh> m_ConeMesh;
    std::unique_ptr<dx12lib::Mesh> m_TorusMesh;
    std::unique_ptr<dx12lib::Mesh> m_PlaneMesh;

    std::shared_ptr<dx12lib::Texture> m_DefaultTexture;
    std::shared_ptr<dx12lib::Texture> m_DirectXTexture;
    std::shared_ptr<dx12lib::Texture> m_EarthTexture;
    std::shared_ptr<dx12lib::Texture> m_MonaLisaTexture;

    std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultTextureView;
    std::shared_ptr<dx12lib::ShaderResourceView> m_DirectXTextureView;
    std::shared_ptr<dx12lib::ShaderResourceView> m_EarthTextureView;
    std::shared_ptr<dx12lib::ShaderResourceView> m_MonaLisaTextureView;

    // Render target
    dx12lib::RenderTarget m_RenderTarget;

    // Root signature
    std::shared_ptr<dx12lib::RootSignature> m_RootSignature;

    // Pipeline state object.
    std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT     m_ScissorRect;

    Camera m_Camera;
    struct alignas( 16 ) CameraData
    {
        DirectX::XMVECTOR m_InitialCamPos;
        DirectX::XMVECTOR m_InitialCamRot;
    };
    CameraData* m_pAlignedCameraData;

    // Camera controller
    float m_Forward;
    float m_Backward;
    float m_Left;
    float m_Right;
    float m_Up;
    float m_Down;

    float m_Pitch;
    float m_Yaw;

    // Rotate the lights in a circle.
    bool m_AnimateLights;
    // Set to true if the Shift key is pressed.
    bool m_Shift;

    int m_Width;
    int m_Height;
    bool m_VSync;

    // Define some lights.
    std::vector<PointLight> m_PointLights;
    std::vector<SpotLight>  m_SpotLights;

    // Logger for logging messages
    Logger m_Logger;
};