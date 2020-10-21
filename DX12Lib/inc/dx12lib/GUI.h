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
 *  @file GUI.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Wrapper for ImGui. This class is used internally by the Window class.
 */

#include "imgui.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>  // For HWND

#include <memory>  // For std::shared_ptr

namespace dx12lib
{

class CommandList;
class Device;
class PipelineStateObject;
class RenderTarget;
class RootSignature;
class ShaderResourceView;
class Texture;

class GUI
{
public:
    /**
     * Window message handler. This needs to be called by the application to allow ImGui to handle input messages.
     */
    LRESULT WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

    /**
     * Begin a new ImGui frame. Do this before calling any ImGui functions that modifies ImGui's render context.
     */
    void NewFrame();

    /**
     * Render ImgGui to the given render target.
     */
    void Render( const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget );

    /**
     * Destroy ImGui context.
     */
    void Destroy();

    /**
     * Set the font scaling for ImGui (this should be called when the window's DPI scaling changes.
     */
    void SetScaling( float scale );

protected:
    GUI( Device& device, HWND hWnd, const RenderTarget& renderTarget );
    virtual ~GUI();

private:
    Device&                              m_Device;
    HWND                                 m_hWnd;
    ImGuiContext*                        m_pImGuiCtx;
    std::shared_ptr<Texture>             m_FontTexture;
    std::shared_ptr<ShaderResourceView>  m_FontSRV;
    std::shared_ptr<RootSignature>       m_RootSignature;
    std::shared_ptr<PipelineStateObject> m_PipelineState;
};
}  // namespace dx12lib