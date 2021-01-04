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
        LMB,    // Is the left-mouse button pressed?
        RMB,    // Is the right-mouse button pressed?
        MoveX,  // Move Left/right.
        MoveY,  // Move Forward/backward.
        MoveZ,  // Move Up/down.
        ZoomIn, // Zoom camera towards focal point.
        ZoomOut,// Zoom camera away from focal point.
        Pitch,  // Look up/down
        Yaw,    // Look left/right.
        Boost,  // Move/look faster
    };

    CameraController( Camera& camera );

    // Reset view to default settings.
    void ResetView();

    // Update the camera based on mouse, keyboard and joystick events.
    // The CameraController assumes that the gainput::InputManger is updated
    // in the main game loop.
    void Update( UpdateEventArgs& e );

    // Whether the pitch should be inverted.
    void SetInverseY( bool inverseY )
    {
        m_InverseY = inverseY;
    }
    bool IsInverseY() const
    {
        return m_InverseY;
    }

private:
    Camera& m_Camera;
    // Keyboard an mouse input.
    std::shared_ptr<gainput::InputMap> m_KMInput;
    // Pad input (separate from Keyboard and mouse input since mouse input is handled differently than pad input)
    std::shared_ptr<gainput::InputMap> m_PadInput;

    Logger m_Logger;

    // Store previous values to apply smoothing.
    float m_X;
    float m_Y;
    float m_Z;
    float m_Zoom;

    // Limit rotation to pitch and yaw.
    float m_Pitch;
    float m_Yaw;
    // Used for smoothing:
    float m_PreviousPitch;
    float m_PreviousYaw;

    bool m_InverseY;
};
