#pragma once

#include <DirectX12Tutorial.h>

class Tutorial1 : public Game
{
public:
    Tutorial1(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowTitle, bool fullscreen = false, bool vsync = true);

protected:
    // Called when starting the application the first time (just before the main
    // update loop.
    virtual void OnInit(EventArgs& e) override;

    // Called when assets should be loaded.
    virtual void OnLoadResources(EventArgs& e) override;

    // Called just before the main update loop.
    virtual void OnStart(EventArgs& e) override;

    // Invoked in the update loop.
    virtual void OnUpdate(UpdateEventArgs& e) override;
    // Invoked when the window should be redrawn.
    virtual void OnRender(RenderEventArgs& e) override;

    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnKeyReleased(KeyEventArgs& e) override;

    virtual void OnWindowClose(WindowCloseEventArgs& e) override;

private:
    // The window used to render the demo.
    std::shared_ptr<Window> m_pWindow;

};