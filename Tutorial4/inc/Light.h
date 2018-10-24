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
 *  @file Light.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Light structures that match HLSL constant buffer padding rules.
 */


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
