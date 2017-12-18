#pragma once

#include <Game.h>
#include <Window.h>

#include <memory>
#include <string>


class Tutorial2 : public Game
{
public:
    Tutorial2(const std::wstring& name, int width, int height, bool vSync = false);

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
    virtual void OnKeyPressed(KeyEventArgs& e);


private:
    using super = Game;
};