#pragma once

#include <DirectXMath.h>

struct PointLight
{
    PointLight()
        : PositionWS( 0.0f, 0.0f, 0.0f, 1.0f )
        , PositionVS( 0.0f, 0.0f, 0.0f, 1.0f )
        , Color( 1.0f, 1.0f, 1.0f, 1.0f )
        , Intensity( 1.0f )
        , Attenuation( 0.0f )
    {}

    DirectX::XMFLOAT4    PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    Color;
    //----------------------------------- (16 byte boundary)
    float       Intensity;
    float       Attenuation;
    float       Padding[2];             // Pad to 16 bytes.
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct SpotLight
{
    SpotLight()
        : PositionWS( 0.0f, 0.0f, 0.0f, 1.0f )
        , PositionVS( 0.0f, 0.0f, 0.0f, 1.0f )
        , DirectionWS( 0.0f, 0.0f, 1.0f, 0.0f )
        , DirectionVS( 0.0f, 0.0f, 1.0f, 0.0f )
        , Color( 1.0f, 1.0f, 1.0f, 1.0f )
        , Intensity( 1.0f )
        , SpotAngle( DirectX::XM_PIDIV2 )
        , Attenuation( 0.0f )
    {}

    DirectX::XMFLOAT4    PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    DirectionWS; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    DirectionVS; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    Color;
    //----------------------------------- (16 byte boundary)
    float       Intensity;
    float       SpotAngle;
    float       Attenuation;
    float       Padding;                // Pad to 16 bytes.
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 6 = 96 bytes
};

struct DirectionalLight
{
    DirectionalLight()
        : DirectionWS( 0.0f, 0.0f, 1.0f, 0.0f )
        , DirectionVS( 0.0f, 0.0f, 1.0f, 0.0f )
        , Color( 1.0f, 1.0f, 1.0f, 1.0f )
    {}

    DirectX::XMFLOAT4    DirectionWS; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    DirectionVS; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4    Color;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 3 = 48 bytes 
};
