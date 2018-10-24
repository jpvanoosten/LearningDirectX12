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
 *  @file Events.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Window and User event structures.
 */

#include "KeyCodes.h"

// Base class for all event args
class EventArgs
{
public:
    EventArgs()
    {}

};

class KeyEventArgs : public EventArgs
{
public:
    enum KeyState
    {
        Released = 0,
        Pressed = 1
    };

    typedef EventArgs base;
    KeyEventArgs( KeyCode::Key key, unsigned int c, KeyState state, bool control, bool shift, bool alt )
        : Key(key)
        , Char(c)
        , State(state)
        , Control(control)
        , Shift(shift)
        , Alt(alt)
    {}

    KeyCode::Key    Key;    // The Key Code that was pressed or released.
    unsigned int    Char;   // The 32-bit character code that was pressed. This value will be 0 if it is a non-printable character.
    KeyState        State;  // Was the key pressed or released?
    bool            Control;// Is the Control modifier pressed
    bool            Shift;  // Is the Shift modifier pressed
    bool            Alt;    // Is the Alt modifier pressed
};

class MouseMotionEventArgs : public EventArgs
{
public:
    typedef EventArgs base;
    MouseMotionEventArgs( bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y )
        : LeftButton( leftButton )
        , MiddleButton( middleButton )
        , RightButton( rightButton )
        , Control( control )
        , Shift( shift )
        , X( x )
        , Y( y )
    {}

    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?

    int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
    int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
    int RelX;           // How far the mouse moved since the last event.
    int RelY;           // How far the mouse moved since the last event.

};

class MouseButtonEventArgs : public EventArgs
{
public:
    enum MouseButton
    {
        None = 0,
        Left = 1,
        Right = 2,
        Middel = 3
    };
    enum ButtonState
    {
        Released = 0,
        Pressed = 1
    };

    typedef EventArgs base;
    MouseButtonEventArgs( MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y )
        : Button( buttonID )
        , State( state )
        , LeftButton( leftButton )
        , MiddleButton( middleButton )
        , RightButton( rightButton )
        , Control( control )
        , Shift( shift )
        , X( x )
        , Y( y )
    {}

    MouseButton Button; // The mouse button that was pressed or released.
    ButtonState State;  // Was the button pressed or released?
    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?

    int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
    int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
};

class MouseWheelEventArgs: public EventArgs
{
public:
    typedef EventArgs base;
    MouseWheelEventArgs( float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y )
        : WheelDelta( wheelDelta )
        , LeftButton( leftButton )
        , MiddleButton( middleButton )
        , RightButton( rightButton )
        , Control( control )
        , Shift( shift )
        , X( x )
        , Y( y )
    {}

    float WheelDelta;   // How much the mouse wheel has moved. A positive value indicates that the wheel was moved to the right. A negative value indicates the wheel was moved to the left.
    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?

    int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
    int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.

};

class ResizeEventArgs : public EventArgs
{
public:
    typedef EventArgs base;
    ResizeEventArgs( int width, int height )
        : Width( width )
        , Height( height )
    {}

    // The new width of the window
    int Width;
    // The new height of the window.
    int Height;

};

class UpdateEventArgs : public EventArgs
{
public:
    typedef EventArgs base;
    UpdateEventArgs( double fDeltaTime, double fTotalTime )
        : ElapsedTime( fDeltaTime )
        , TotalTime( fTotalTime )
    {}

    double ElapsedTime;
    double TotalTime;
};

class RenderEventArgs : public EventArgs
{
public:
    typedef EventArgs base;
    RenderEventArgs( double fDeltaTime, double fTotalTime )
        : ElapsedTime( fDeltaTime )
        , TotalTime( fTotalTime )
    {}

    double ElapsedTime;
    double TotalTime;
};

class UserEventArgs : public EventArgs
{
public:
    typedef EventArgs base;
    UserEventArgs( int code, void* data1, void* data2 )
        : Code( code )
        , Data1( data1 )
        , Data2( data2 )
    {}

    int     Code;
    void*   Data1;
    void*   Data2;
};