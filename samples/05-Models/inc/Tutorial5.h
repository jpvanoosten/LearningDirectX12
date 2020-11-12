#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
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
 *  @file Tutorial5.h
 *  @date October 29, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief In this tutorial, we'll load a model.
 */

#include "Camera.h"
#include "CameraController.h"
#include "Light.h"

#include <GameFramework/GameFramework.h>

#include <dx12lib/RenderTarget.h>

#include <d3d12.h>  // For D3D12_RECT

#include <future>  // For std::future.
#include <memory>
#include <string>

namespace dx12lib
{
class CommandList;
class Device;
class GUI;
class PipelineStateObject;
class RenderTarget;
class RootSignature;
class Scene;
class SwapChain;
}  // namespace dx12lib

class BasicLightingPSO;

class Tutorial5
{
public:
    Tutorial5( const std::wstring& name, int width, int height, bool vSync = false );
    ~Tutorial5();

    /**
     * Start the main game loop.
     */
    uint32_t Run();

    /**
     * Load content requred for the demo.
     */
    void LoadContent();

    /**
     * Unload content that was loaded in LoadContent.
     */
    void UnloadContent();

protected:
    /**
     * Update game logic.
     */
    void OnUpdate( UpdateEventArgs& e );

    /**
     * Window is being resized.
     */
    void OnResize( ResizeEventArgs& e );

    /**
     * Render stuff.
     */
    void OnRender();

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    void OnKeyPressed( KeyEventArgs& e );

    /**
     * Invoked when a key on the keyboard is released.
     */
    void OnKeyReleased( KeyEventArgs& e );

    /**
     * Invoked when the mouse is moved over the registered window.
     */
    virtual void OnMouseMoved( MouseMotionEventArgs& e );

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    void OnMouseWheel( MouseWheelEventArgs& e );

    /**
     * Handle DPI change events.
     */
    void OnDPIScaleChanged( DPIScaleEventArgs& e );

    /**
     * Render ImGUI stuff.
     */
    void OnGUI( const std::shared_ptr<dx12lib::CommandList>& commandList, const dx12lib::RenderTarget& renderTarget );

private:
    /**
     * Load all of the assets (scene file, shaders, etc...).
     * This is executed as an async task so that we can render a loading screen in the main thread.
     */
    bool LoadAssets();

    /**
     * This function is called to report the loading progress of the scene. This is useful for updating the loading
     * progress bar.
     *
     * @param progress The loading progress (as a normalized float in the range [0...1].
     *
     * @returns true to continue loading or false to cancel loading.
     */
    bool LoadingProgress( float loadingProgress );

    std::shared_ptr<dx12lib::Device>    m_Device;
    std::shared_ptr<dx12lib::SwapChain> m_SwapChain;
    std::shared_ptr<dx12lib::GUI>       m_GUI;

    std::shared_ptr<dx12lib::Scene> m_Scene;

    // Pipeline state object for rendering the scene.
    std::shared_ptr<BasicLightingPSO> m_PSO;

    // Render target
    dx12lib::RenderTarget m_RenderTarget;

    std::shared_ptr<Window> m_Window;

    D3D12_RECT     m_ScissorRect;
    D3D12_VIEWPORT m_Viewport;

    Camera           m_Camera;
    CameraController m_CameraController;
    Logger           m_Logger;

    int  m_Width;
    int  m_Height;
    bool m_VSync;

    // Define some lights.
    std::vector<PointLight> m_PointLights;
    std::vector<SpotLight>  m_SpotLights;
    std::vector<DirectionalLight> m_DirectionalLights;

    // Rotate the lights in a circle.
    bool m_AnimateLights;

    bool              m_Fullscreen;
    bool              m_AllowFullscreenToggle;
    std::atomic_bool  m_IsLoading;
    std::future<bool> m_LoadingTask;
    float             m_LoadingProgress;
    std::string       m_LoadingText;
};