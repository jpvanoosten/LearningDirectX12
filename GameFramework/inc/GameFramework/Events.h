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
 *  @file Events.h
 *  @date September 29, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Application and Window events.
 */

#include "KeyCodes.h"

#include "../signals/signals.hpp"

#include <string>

/**
 * A Delegate holds function callbacks.
 */

// Primary delegate template.
template<typename Func>
class Delegate;

template<typename R, typename... Args>
class Delegate<R( Args... )>
{
public:
    using slot              = sig::slot<R( Args... )>;
    using signal            = sig::signal<R( Args... )>;
    using connection        = sig::connection;
    using scoped_connection = sig::scoped_connection;

    /**
     * Add function callback to the delegate.
     *
     * @param f The function to add to the delegate.
     * @returns The connection object that can be used to remove the callback
     * from the delegate.
     */
    template<typename Func>
    connection operator+=( Func&& f )
    {
        return m_Callbacks.connect( std::forward<Func>( f ) );
    }

    /**
     * Remove a callback function from the delegate.
     *
     * @param f The function to remove from the delegate.
     * @returns The number of callback functions removed.
     */
    template<typename Func>
    std::size_t operator-=( Func&& f )
    {
        return m_Callbacks.disconnect( std::forward<Func>( f ) );
    }

    /**
     * Invoke the delegate.
     */
    opt::optional<R> operator()( Args... args )
    {
        return m_Callbacks( std::forward<Args>( args )... );
    }

private:
    signal m_Callbacks;
};

/**
 * Base class for all event args.
 */
class EventArgs
{
public:
    EventArgs() = default;
};

// Define the default event.
using Event = Delegate<void( EventArgs& )>;

/**
 * Update event args.
 */
class UpdateEventArgs : public EventArgs
{
public:
    UpdateEventArgs( double deltaTime, double totalTime )
    : DeltaTime( deltaTime )
    , TotalTime( totalTime )
    {}

    // The elapsed time (in seconds)
    double DeltaTime;
    // Total time the application has been running (in seconds).
    double TotalTime;
};

using UpdateEvent = Delegate<void( UpdateEventArgs& )>;

/**
 * Render event args.
 */
class RenderEventArgs : public EventArgs
{
public:
    RenderEventArgs() {}
};

using RenderEvent = Delegate<void( RenderEventArgs& )>;

class DPIScaleEventArgs : public EventArgs
{
public:
    typedef EventArgs base;
    DPIScaleEventArgs( float dpiScale )
    : DPIScale( dpiScale )
    {}

    float DPIScale;
};

using DPIScaleEvent = Delegate<void( DPIScaleEventArgs& )>;

/**
 * EventArgs for a WindowClosing event.
 */
class WindowCloseEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    WindowCloseEventArgs()
    : base()
    , ConfirmClose( true )
    {}

    /**
     * The user can cancel a window closing operation by registering for the
     * Window::Close event on the Window and setting the
     * WindowCloseEventArgs::ConfirmClose property to false if the window should
     * be kept open (for example, if closing the window would cause unsaved
     * file changes to be lost).
     */
    bool ConfirmClose;
};

using WindowCloseEvent = Delegate<void( WindowCloseEventArgs& )>;

enum class KeyState
{
    Released = 0,
    Pressed  = 1,
};

/**
 * KeyEventArgs are used for keyboard key pressed/released events.
 */
class KeyEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    KeyEventArgs( KeyCode key, unsigned int c, KeyState state, bool control, bool shift, bool alt )
    : base()
    , Key( key )
    , Char( c )
    , State( state )
    , Control( control )
    , Shift( shift )
    , Alt( alt )
    {}

    KeyCode      Key;   // The Key Code that was pressed or released.
    unsigned int Char;  // The 32-bit character code that was pressed. This
                        // value will be 0 if it is a non-printable character.
    KeyState State;     // Was the key pressed or released?
    bool     Control;   // Is the Control modifier pressed
    bool     Shift;     // Is the Shift modifier pressed
    bool     Alt;       // Is the Alt modifier pressed
};

using KeyboardEvent = Delegate<void( KeyEventArgs& )>;

/**
 * MouseMotionEventArgs are used to indicate the mouse moved or was dragged over
 * the window.
 */
class MouseMotionEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    MouseMotionEventArgs( bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y )
    : base()
    , LeftButton( leftButton )
    , MiddleButton( middleButton )
    , RightButton( rightButton )
    , Control( control )
    , Shift( shift )
    , X( x )
    , Y( y )
    , RelX( 0 )
    , RelY( 0 )
    {}

    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?
    int  X;             // The X-position of the cursor relative to the upper-left corner of
                        // the client area (in pixels).
    int Y;              // The Y-position of the cursor relative to the upper-left corner of
                        // the client area (in pixels).
    int RelX;           // How far the mouse moved since the last event (in pixels).
    int RelY;           // How far the mouse moved since the last event (in pixels).
};

using MouseMotionEvent = Delegate<void( MouseMotionEventArgs& )>;

enum class MouseButton
{
    None   = 0,
    Left   = 1,
    Right  = 2,
    Middle = 3
};

enum class ButtonState
{
    Released = 0,
    Pressed  = 1
};

class MouseButtonEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    MouseButtonEventArgs( MouseButton button, ButtonState state, bool leftButton, bool middleButton, bool rightButton,
                          bool control, bool shift, int x, int y )
    : base()
    , Button( button )
    , State( state )
    , LeftButton( leftButton )
    , MiddleButton( middleButton )
    , RightButton( rightButton )
    , Control( control )
    , Shift( shift )
    , X( x )
    , Y( y )
    {}

    MouseButton Button;        // The mouse button that was pressed or released.
    ButtonState State;         // Was the button pressed or released?
    bool        LeftButton;    // Is the left mouse button down?
    bool        MiddleButton;  // Is the middle mouse button down?
    bool        RightButton;   // Is the right mouse button down?
    bool        Control;       // Is the CTRL key down?
    bool        Shift;         // Is the Shift key down?

    int X;  // The X-position of the cursor relative to the upper-left corner of
            // the client area.
    int Y;  // The Y-position of the cursor relative to the upper-left corner of
            // the client area.
};

using MouseButtonEvent = Delegate<void( MouseButtonEventArgs& )>;

/**
 * MouseWheelEventArgs indicates if the mouse wheel was moved and how much.
 */
class MouseWheelEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    MouseWheelEventArgs( float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control,
                         bool shift, int x, int y )
    : base()
    , WheelDelta( wheelDelta )
    , LeftButton( leftButton )
    , MiddleButton( middleButton )
    , RightButton( rightButton )
    , Control( control )
    , Shift( shift )
    , X( x )
    , Y( y )
    {}

    float WheelDelta;   // How much the mouse wheel has moved. A positive value
                        // indicates that the wheel was moved to the right. A
                        // negative value indicates the wheel was moved to the
                        // left.
    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?
    int  X;             // The X-position of the cursor relative to the upper-left corner of
                        // the client area.
    int Y;              // The Y-position of the cursor relative to the upper-left corner of
                        // the client area.
};
using MouseWheelEvent = Delegate<void( MouseWheelEventArgs& )>;

enum class WindowState
{
    Restored  = 0,  // The window has been resized.
    Minimized = 1,  // The window has been minimized.
    Maximized = 2,  // The window has been maximized.
};

/**
 * Event args to indicate the window has been resized.
 */
class ResizeEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    ResizeEventArgs( int width, int height, WindowState state )
    : base()
    , Width( width )
    , Height( height )
    , State( state )
    {}

    // The new width of the window
    int Width;
    // The new height of the window.
    int Height;
    // If the window was minimized or maximized.
    WindowState State;
};
using ResizeEvent = Delegate<void( ResizeEventArgs& )>;

/**
 * Generic user event args.
 */
class UserEventArgs : public EventArgs
{
public:
    using base = EventArgs;
    UserEventArgs( int code, void* data1, void* data2 )
    : base()
    , Code( code )
    , Data1( data1 )
    , Data2( data2 )
    {}

    int   Code;
    void* Data1;
    void* Data2;
};
using UserEvent = Delegate<void( UserEventArgs& )>;

/**
 * Used to notify a runtime error occurred.
 */
class RuntimeErrorEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    RuntimeErrorEventArgs( const std::string& errorString, const std::string& compilerError )
    : base()
    , ErrorString( errorString )
    , CompilerError( compilerError )
    {}

    std::string ErrorString;
    std::string CompilerError;
};
using RuntimeErrorEvent = Delegate<void( RuntimeErrorEventArgs& )>;

enum class FileAction
{
    Unknown,    // An unknown action triggered this event. (Should not happen, but
                // I guess it's possible)
    Added,      // A file was added to a directory.
    Removed,    // A file was removed from a directory.
    Modified,   // A file was modified. This might indicate the file's timestamp
                // was modified or another attribute was modified.
    RenameOld,  // The file was renamed and this event stores the previous name.
    RenameNew,  // The file was renamed and this event stores the new name.
};

class FileChangedEventArgs : public EventArgs
{
public:
    using base = EventArgs;

    FileChangedEventArgs( FileAction action, const std::wstring& path )
    : base()
    , Action( action )
    , Path( path )
    {}

    FileAction   Action;  // The action that triggered this event.
    std::wstring Path;    // The file or directory path that was modified.
};
using FileChangeEvent = Delegate<void( FileChangedEventArgs& )>;
