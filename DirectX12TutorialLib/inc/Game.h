/*
 *  Copyright(c) 2017 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
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
 *  @file DX12Game.h
 *  @date August 25, 2017
 *  @author Jeremiah van Oosten
 *
 *  @brief The base class for a DirectX 12 game class.
 */

#pragma once

#include "DirectX12TutorialDefines.h"

class Window;

class DX12TL_DLL Game
{
public:
    Game(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle, bool fullscreen = false, bool vsync = true );
    virtual ~Game();

    uint32_t GetWindowWidth() const;
    uint32_t GetWindowHeight() const;

    const std::wstring& GetWindowTitle() const;

protected:
     // Called when starting the application the first time (just before the main
     // update loop.
    virtual void OnInit(EventArgs& e);
    
    // Called when assets should be loaded.
    virtual void OnLoadResources(EventArgs& e);

    // Called just before the main update loop.
    virtual void OnStart(EventArgs& e);

    // Invoked in the update loop.
    virtual void OnUpdate(UpdateEventArgs& e);
    // Invoked when the window should be redrawn.
    virtual void OnRender(RenderEventArgs& e);

    virtual void OnKeyPressed(KeyEventArgs& e);
    virtual void OnKeyReleased(KeyEventArgs& e);

    virtual void OnWindowClose(WindowCloseEventArgs& e);
private:

    // The window used to render the demo.
    std::shared_ptr<Window> m_pWindow;
};
