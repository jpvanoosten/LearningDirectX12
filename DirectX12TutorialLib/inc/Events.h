#pragma once

/*
 *  Copyright(c) 2015 Jeremiah van Oosten
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
  *  @date December 21, 2015
  *  @author Jeremiah
  *
  *  @brief Simple event library that wraps the boost::signals2 library.
  */

#include "DirectX12TutorialDefines.h"
#include "KeyCodes.h"

// #include <boost/signals2.hpp>

class Object;

// Delegate template class for encapsulating event callback functions.
template< typename... ArgumentTypes >
class DX12TL_DLL Delegate
{
public:
    using FunctionType = std::function< void(ArgumentTypes...) >;
    using ConnectionType = boost::signals2::connection;

    // Using scoped connections can help to manage connection lifetimes.
    using ScopedConnections = std::vector< boost::signals2::scoped_connection >;

    // Add a callback to the the list
    // Returns the connection object that can be used to disconnect the 
    // subscriber from the signal.
    ConnectionType operator += (typename const FunctionType& callback) const
    {
        return m_Callbacks.connect(callback);
    }

    // Remove a callback from the list
    void operator -= (typename const FunctionType& callback) const
    {
        assert(false);
        // TODO: This isn't working yet.. Getting a compiler error:
        // Error	1	error C2666: 'boost::operator ==' : 4 overloads have similar conversions signal_template.hpp
        // WORKAROUND: Use the connection object returned when the subscriber was initially connected
        // to disconnect the subscriber.
//        m_Callbacks.disconnect<FunctionType>( callback );
    }

    void operator -= (ConnectionType& con)
    {
        m_Callbacks.disconnect(con); // This doesn't seem to work either!?
        if (con.connected())
        {
            con.disconnect(); // This is stupid, then any connection passed to this function will be disconnected, even if it was never registered with this signal.
        }
    }

    // Invoke this event with the argument
    void operator()(typename ArgumentTypes... arguments)
    {
        m_Callbacks(arguments...);
    }

private:
    using Callbacks = boost::signals2::signal< void(ArgumentTypes...) >;
    mutable Callbacks   m_Callbacks;
};

// Base class for all event args
class EventArgs
{
public:
    EventArgs(Object& caller)
        : Caller(caller)
    {}

    // The object that invoked the event
    Object& Caller;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<EventArgs&>;
using Event = Delegate<EventArgs&>;

class WindowCloseEventArgs : EventArgs
{
public:
    using base = EventArgs;
    WindowCloseEventArgs(Object& caller)
        : base(caller)
        , ConfirmClose(true)
    {}

    // The user can cancel a window closing operating by registering for the 
    // Window::Close event on the window and setting the ConfirmClose to false if
    // the window should be kept open (for example, there are unsaved changes 
    // made and closing the window would cause those changes to be lost).
    // By default, the window will be destoryed if the Window::Close event is not handled.
    bool ConfirmClose;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<WindowCloseEventArgs&>;
using WindowCloseEvent = Delegate<WindowCloseEventArgs&>;

enum class KeyState
{
    Released = 0,
    Pressed = 1
};

class KeyEventArgs : public EventArgs
{
public:

    using base = EventArgs;
    KeyEventArgs(Object& caller, KeyCode key, unsigned int c, KeyState state, bool control, bool shift, bool alt)
        : base(caller)
        , Key(key)
        , Char(c)
        , State(state)
        , Control(control)
        , Shift(shift)
        , Alt(alt)
    {}

    KeyCode         Key;    // The Key Code that was pressed or released.
    unsigned int    Char;   // The 32-bit character code that was pressed. This value will be 0 if it is a non-printable character.
    KeyState        State;  // Was the key pressed or released?
    bool            Control;// Is the Control modifier pressed
    bool            Shift;  // Is the Shift modifier pressed
    bool            Alt;    // Is the Alt modifier pressed
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<KeyEventArgs&>;
using KeyboardEvent = Delegate<KeyEventArgs&>;

class MouseMotionEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    MouseMotionEventArgs(Object& caller, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
        : base(caller)
        , LeftButton(leftButton)
        , MiddleButton(middleButton)
        , RightButton(rightButton)
        , Control(control)
        , Shift(shift)
        , X(x)
        , Y(y)
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
DX12TL_EXTERN template class DX12TL_DLL Delegate<MouseMotionEventArgs&>;
using MouseMotionEvent = Delegate<MouseMotionEventArgs&>;

enum class MouseButton
{
    None = 0,
    Left = 1,
    Right = 2,
    Middel = 3
};

enum class ButtonState
{
    Released = 0,
    Pressed = 1
};

class MouseButtonEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    MouseButtonEventArgs(Object& caller, MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
        : base(caller)
        , Button(buttonID)
        , State(state)
        , LeftButton(leftButton)
        , MiddleButton(middleButton)
        , RightButton(rightButton)
        , Control(control)
        , Shift(shift)
        , X(x)
        , Y(y)
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
DX12TL_EXTERN template class DX12TL_DLL Delegate<MouseButtonEventArgs&>;
using MouseButtonEvent = Delegate<MouseButtonEventArgs&>;

class MouseWheelEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    MouseWheelEventArgs(Object& caller, float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
        : base(caller)
        , WheelDelta(wheelDelta)
        , LeftButton(leftButton)
        , MiddleButton(middleButton)
        , RightButton(rightButton)
        , Control(control)
        , Shift(shift)
        , X(x)
        , Y(y)
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
DX12TL_EXTERN template class DX12TL_DLL Delegate<MouseWheelEventArgs&>;
using MouseWheelEvent = Delegate<MouseWheelEventArgs&>;

class JoystickButtonEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    JoystickButtonEventArgs(Object& caller, unsigned int joystickID, ButtonState state, unsigned int buttonID, bool buttonStates[32])
        : base(caller)
        , JoystickID(joystickID)
        , State(state)
        , ButtonID(buttonID)
    {
        memcpy_s(ButtonStates, sizeof(ButtonStates), buttonStates, sizeof(buttonStates));
    }

    // The ID of the joystick that triggered this event.
    // Value is in the range [0 .. 15].
    unsigned int JoystickID;
    // Was the button pressed or released?
    ButtonState State;
    // The 0-based index ID of the button that was pressed or released.
    // Button1 is index 0, button2 is index 1, etc...
    unsigned int ButtonID;

    // The state of the joystick buttons when this event was invoked.
    bool ButtonStates[32];
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<JoystickButtonEventArgs&>;
using JoystickButtonEvent = Delegate<JoystickButtonEventArgs&>;

enum class POVDirection : int
{
    Centered = -1,
    Up = 0,
    UpRight = 45,
    Right = 90,
    DownRight = 135,
    Down = 180,
    DownLeft = 225,
    Left = 270,
    UpLeft = 315,
};

class JoystickPOVEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    JoystickPOVEventArgs(Object& caller, unsigned int joystickID, float povAngle, POVDirection povDirection, bool buttonStates[32])
        : base(caller)
        , JoystickID(joystickID)
        , Angle(povAngle)
        , Direction(povDirection)
    {
        memcpy_s(ButtonStates, sizeof(ButtonStates), buttonStates, sizeof(buttonStates));
    }

    // The ID of the joystick that triggered this event.
    // Value is in the range [0 .. 15].
    unsigned int JoystickID;

    // The angle in degrees of the Point of View Hat.
    // The angle is similar to that of a clock. At 12:00, the angle is 0 degrees,
    // at 3:00, the angle is 90 degrees, at 6:00, the angle is 180 degrees,
    // and at 9:00 the angle is 270 degrees.
    // The angle is set to -1.0f if the PoV hat is centered.
    float Angle;

    // Discrete direction of the POV.
    POVDirection Direction;

    // The state of the joystick buttons when this event was invoked.
    bool ButtonStates[32];
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<JoystickPOVEventArgs&>;
using JoystickPOVEvent = Delegate<JoystickPOVEventArgs&>;

enum class JoystickAxis
{
    RAxis,
    UAxis,
    VAxis,
    XAxis,
    YAxis,
    ZAxis,
};

class JoystickAxisEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    JoystickAxisEventArgs(Object& caller, unsigned int joystickID, JoystickAxis changedAxis, float axis, bool buttonStates[32])
        : base(caller)
        , JoystickID(joystickID)
        , ChangedAxis(changedAxis)
        , Axis(axis)
    {
        memcpy_s(ButtonStates, sizeof(ButtonStates), buttonStates, sizeof(buttonStates));
    }

    // The ID of the joystick that triggered this event.
    // Value is in the range [0 .. 15].
    unsigned int JoystickID;

    // The joystick axis that changed and generated this event.
    JoystickAxis ChangedAxis;

    // The value of the axis that changed.
    // This value will be in the range [-1 .. 1]
    float Axis;

    // The buttons states when this event was fired.
    bool ButtonStates[32];
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<JoystickAxisEventArgs&>;
using JoystickAxisEvent = Delegate<JoystickAxisEventArgs&>;

// The type of action that cause the resize event
enum class ResizeAction
{
    Resized,    // The window size is changed by dragging the window frame.
    Minimized,  // The window is being minimized.
    Maximized,  // The window is being maximized.
};

class ResizeEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    ResizeEventArgs(Object& caller, int width, int height, ResizeAction action )
        : base(caller)
        , Width(width)
        , Height(height)
        , Action(action)
    {}

    // The new width of the window
    int Width;
    // The new height of the window.
    int Height;
    // The action that triggered the resize event.
    ResizeAction Action;

};
DX12TL_EXTERN template class DX12TL_DLL Delegate<ResizeEventArgs&>;
using ResizeEvent = Delegate<ResizeEventArgs&>;

class UpdateEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    UpdateEventArgs(Object& caller, double fDeltaTime, double fTotalTime, uint64_t frameCounter)
        : base(caller)
        , ElapsedTime(fDeltaTime)
        , TotalTime(fTotalTime)
        , FrameCounter(frameCounter)
    {}

    double ElapsedTime;
    double TotalTime;
    uint64_t FrameCounter;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<UpdateEventArgs&>;
using UpdateEvent = Delegate<UpdateEventArgs&>;

class RenderEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    RenderEventArgs(
        Object& caller, 
        double fDeltaTime, 
        double fTotalTime,
        uint64_t frameCounter)
        //std::shared_ptr<Graphics::Camera> camera = nullptr,
        //std::shared_ptr<Graphics::GraphicsCommandBuffer> graphicsCommandBuffer = nullptr)
        : base(caller)
        , ElapsedTime(fDeltaTime)
        , TotalTime(fTotalTime)
        , FrameCounter(frameCounter)
        //, Camera(camera)
        //, GraphicsCommandBuffer(graphicsCommandBuffer)
    {}

    double ElapsedTime;
    double TotalTime;
    uint64_t FrameCounter;

    //std::shared_ptr<Graphics::Camera> Camera;
    //std::shared_ptr<Graphics::GraphicsCommandBuffer> GraphicsCommandBuffer;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<RenderEventArgs&>;
using RenderEvent = Delegate<RenderEventArgs&>;

class UserEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    UserEventArgs(Object& caller, int code, void* data1, void* data2)
        : base(caller)
        , Code(code)
        , Data1(data1)
        , Data2(data2)
    {}

    int     Code;
    void*   Data1;
    void*   Data2;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<UserEventArgs&>;
using UserEvent = Delegate<UserEventArgs&>;

class RuntimeErrorEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    RuntimeErrorEventArgs(Object& caller, const std::string& errorString, const std::string& compilerError)
        : base(caller)
        , ErrorString(errorString)
        , CompilerError(compilerError)
    {}

    std::string ErrorString;
    std::string CompilerError;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<RuntimeErrorEventArgs&>;
using RuntimeErrorEvent = Delegate<RuntimeErrorEventArgs&>;

class ProgressEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    ProgressEventArgs(Object& caller, const std::wstring& fileName, float progress, bool cancel = false)
        : base(caller)
        , FileName(fileName)
        , Progress(progress)
        , Cancel(cancel)
    {}

    // The file that is being loaded.
    std::wstring FileName;
    // The progress of the loading process.
    float Progress;
    // Set to TRUE to cancel loading.
    bool Cancel;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<ProgressEventArgs&>;
using ProgressEvent = Delegate<ProgressEventArgs&>;

enum class FileAction
{
    Unknown,        // An unknown action triggered this event. (Should not happen, but I guess it's possible)
    Added,          // A file was added to a directory.
    Removed,        // A file was removed from a directory.
    Modified,       // A file was modified. This might indicate the file's timestamp was modified or another attribute was modified.
    RenameOld,      // The file was renamed and this event stores the previous name.
    RenameNew,      // The file was renamed and this event stores the new name.
};

class FileChangeEventArgs : EventArgs
{
public:
    using base = EventArgs;

    FileChangeEventArgs(Object& caller, FileAction action, const std::wstring& path)
        : base(caller)
        , Action(action)
        , Path(path)
    {}

    FileAction Action; // The action that triggered this event.
                       // The file or directory path that was modified.
    std::wstring Path;
};
DX12TL_EXTERN template class DX12TL_DLL Delegate<FileChangeEventArgs&>;
using FileChangeEvent = Delegate<FileChangeEventArgs&>;
