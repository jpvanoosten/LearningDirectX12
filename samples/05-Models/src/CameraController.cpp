#include <CameraController.h>

#include <Camera.h>

CameraController::CameraController( Camera& camera )
: m_Camera( camera )
{
    auto& gf = GameFramework::Get();

    m_Logger = gf.CreateLogger( "CameraController" );

    m_InputMap = gf.CreateInputMap( "CameraController" );

    auto keyboard = gf.GetKeyboardId();
    auto mouse    = gf.GetMouseId();
    auto pad      = gf.GetPadId( 0 );  // Just use the first connected device.

    // Map keyboard events.
    m_InputMap->MapFloat( MoveX, keyboard, gainput::KeyD, 0.0f, 1.0f );
    m_InputMap->MapFloat( MoveX, keyboard, gainput::KeyA, 0.0f, -1.0f );
    m_InputMap->MapFloat( MoveX, keyboard, gainput::KeyRight, 0.0f, 1.0f );
    m_InputMap->MapFloat( MoveX, keyboard, gainput::KeyLeft, 0.0f, -1.0f );
    m_InputMap->MapFloat( MoveY, keyboard, gainput::KeyW, 0.0f, 1.0f );
    m_InputMap->MapFloat( MoveY, keyboard, gainput::KeyS, 0.0f, -1.0f );
    m_InputMap->MapFloat( MoveY, keyboard, gainput::KeyUp, 0.0f, 1.0f );
    m_InputMap->MapFloat( MoveY, keyboard, gainput::KeyDown, 0.0f, -1.0f );
    m_InputMap->MapFloat( MoveZ, keyboard, gainput::KeyE, 0.0f, 1.0f );
    m_InputMap->MapFloat( MoveZ, keyboard, gainput::KeyQ, 0.0f, -1.0f );
    m_InputMap->MapBool( Boost, keyboard, gainput::KeyShiftL );
    m_InputMap->MapBool( Boost, keyboard, gainput::KeyShiftR );

    // Map mouse events
    m_InputMap->MapFloat( Pitch, mouse, gainput::MouseAxisY );
    m_InputMap->MapFloat( Yaw, mouse, gainput::MouseAxisX );

    // Map pad events.
    m_InputMap->MapFloat( MoveX, pad, gainput::PadButtonLeftStickX );
    m_InputMap->MapFloat( MoveY, pad, gainput::PadButtonLeftStickY );
    m_InputMap->MapFloat( MoveZ, pad, gainput::PadButtonAxis4, 0.0f, -1.0f );  // Left trigger (move down)
    m_InputMap->MapFloat( MoveZ, pad, gainput::PadButtonAxis5, 0.0f, 1.0f );   // Right trigger (move up)
    m_InputMap->MapFloat( Pitch, pad, gainput::PadButtonRightStickY );
    m_InputMap->MapFloat( Yaw, pad, gainput::PadButtonRightStickX );
    m_InputMap->MapBool( Boost, pad, gainput::PadButtonL3 );

    // Set button policies for actions that map to multiple devices.
    m_InputMap->SetUserButtonPolicy( MoveX, gainput::InputMap::UBP_MAX );
    m_InputMap->SetUserButtonPolicy( MoveY, gainput::InputMap::UBP_MAX );
    m_InputMap->SetUserButtonPolicy( MoveZ, gainput::InputMap::UBP_MAX );
    m_InputMap->SetUserButtonPolicy( Pitch, gainput::InputMap::UBP_MAX );
    m_InputMap->SetUserButtonPolicy( Yaw, gainput::InputMap::UBP_MAX );
}

void CameraController::Update( UpdateEventArgs& e )
{
    if ( m_InputMap->GetFloatPrevious( MoveX ) != m_InputMap->GetFloat( MoveX ) )
    {
        m_Logger->info( "MoveX: {}", m_InputMap->GetFloat( MoveX ) );
    }
    if ( m_InputMap->GetFloatPrevious( MoveY ) != m_InputMap->GetFloat( MoveY ) )
    {
        m_Logger->info( "MoveY: {}", m_InputMap->GetFloat( MoveY ) );
    }
    if ( m_InputMap->GetFloatPrevious( MoveZ ) != m_InputMap->GetFloat( MoveZ ) )
    {
        m_Logger->info( "MoveZ: {}", m_InputMap->GetFloat( MoveZ ) );
    }
    if ( m_InputMap->GetFloatPrevious( Pitch ) != m_InputMap->GetFloat( Pitch ) )
    {
        m_Logger->info( "Pitch: {}", m_InputMap->GetFloatDelta( Pitch ) );
    }
    if ( m_InputMap->GetFloatPrevious( Yaw ) != m_InputMap->GetFloat( Yaw ) )
    {
        m_Logger->info( "Yaw: {}", m_InputMap->GetFloatDelta( Yaw ) );
    }
    if ( m_InputMap->GetBoolIsNew( Boost ) )
    {
        m_Logger->info( "Boost!" );
    }
}
