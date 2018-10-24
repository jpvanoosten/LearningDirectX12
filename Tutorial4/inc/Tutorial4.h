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
 *  @file Tutorial4.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Tutorial 4.
 */


#include <Camera.h>
#include <Game.h>
#include <IndexBuffer.h>
#include <Light.h>
#include <Window.h>
#include <Mesh.h>
#include <RenderTarget.h>
#include <RootSignature.h>
#include <Texture.h>
#include <VertexBuffer.h>

#include <DirectXMath.h>

class Tutorial4 : public Game
{
public:
    using super = Game;

    Tutorial4(const std::wstring& name, int width, int height, bool vSync = false);
    virtual ~Tutorial4();

    /**
     *  Load content required for the demo.
     */
    virtual bool LoadContent() override;

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    virtual void UnloadContent() override;
protected:
    /**
     *  Update the game logic.
     */
    virtual void OnUpdate(UpdateEventArgs& e) override;

    /**
     *  Render stuff.
     */
    virtual void OnRender(RenderEventArgs& e) override;

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    virtual void OnKeyPressed(KeyEventArgs& e) override;

    /**
     * Invoked when a key on the keyboard is released.
     */
    virtual void OnKeyReleased(KeyEventArgs& e);

    /**
     * Invoked when the mouse is moved over the registered window.
     */
    virtual void OnMouseMoved(MouseMotionEventArgs& e);

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;


    virtual void OnResize(ResizeEventArgs& e) override; 

private:
    // Some geometry to render.
    std::unique_ptr<Mesh> m_CubeMesh;
    std::unique_ptr<Mesh> m_SphereMesh;
    std::unique_ptr<Mesh> m_ConeMesh;
    std::unique_ptr<Mesh> m_TorusMesh;
    std::unique_ptr<Mesh> m_PlaneMesh;

    std::unique_ptr<Mesh> m_SkyboxMesh;

    Texture m_DefaultTexture;
    Texture m_DirectXTexture;
    Texture m_EarthTexture;
    Texture m_MonaLisaTexture;
    Texture m_GraceCathedralTexture;
    Texture m_GraceCathedralCubemap;

    // HDR Render target
    RenderTarget m_HDRRenderTarget;

    // Root signatures
    RootSignature m_SkyboxSignature;
    RootSignature m_HDRRootSignature;
    RootSignature m_SDRRootSignature;

    // Pipeline state object.
    // Skybox PSO
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_SkyboxPipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_HDRPipelineState;
    // HDR -> SDR tone mapping PSO.
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_SDRPipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

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

    // Define some lights.
    std::vector<PointLight> m_PointLights;
    std::vector<SpotLight> m_SpotLights;
};