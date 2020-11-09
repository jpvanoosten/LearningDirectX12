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
 *  @file CameraController.h
 *  @date November 9, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Uses mouse, keyboard and joystick input to update the camera.
 */

#include <GameFramework/GameFramework.h>  // For Logger

#include <memory>  // For std::shared_ptr

// Our camera controller uses an gainput::InputMap to map events to device buttons.
// @see
namespace gainput
{
class InputMap;
}

class Camera;

class CameraController
{
public:
    enum Actions
    {
        MoveX,  // Move Left/right.
        MoveY,  // Move Forward/backward.
        MoveZ,  // Move Up/down.
        Pitch,  // Look up/down
        Yaw,    // Look left/right.
        Boost,  // Move/look faster
    };

    CameraController( Camera& camera );

    // Update the camera based on mouse, keyboard and joystick events.
    // The CameraController assumes that the gainput::InputManger is updated
    // in the main game loop.
    void Update( UpdateEventArgs& e );

private:
    Camera&                            m_Camera;
    std::shared_ptr<gainput::InputMap> m_InputMap;

    Logger m_Logger;
};
