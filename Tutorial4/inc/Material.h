#pragma once
#include <DirectXMath.h>

struct Material
{
    Material(
        DirectX::XMFLOAT4 emissive = { 0.0f, 0.0f, 0.0f, 1.0f },
        DirectX::XMFLOAT4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f },
        DirectX::XMFLOAT4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f },
        DirectX::XMFLOAT4 specular = { 1.0f, 1.0f, 1.0f, 1.0f },
        float specularPower = 128.0f
    )
        : Emissive( emissive )
        , Ambient( ambient )
        , Diffuse( diffuse )
        , Specular( specular )
        , SpecularPower( specularPower ) 
    {}

    DirectX::XMFLOAT4 Emissive;
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4 Ambient;
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4 Diffuse;
    //----------------------------------- (16 byte boundary)
    DirectX::XMFLOAT4 Specular;
    //----------------------------------- (16 byte boundary)
    float             SpecularPower;
    uint32_t          Padding[3];
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 5 = 80 bytes
    
    // Define some interesting materials.
    static const Material Red;
    static const Material Green;
    static const Material Blue;
    static const Material Cyan;
    static const Material Magenta;
    static const Material Yellow;
    static const Material White;
    static const Material Black;
    static const Material Emerald;
    static const Material Jade;
    static const Material Obsidian;
    static const Material Pearl;
    static const Material Ruby;
    static const Material Turquoise;
    static const Material Brass;
    static const Material Bronze;
    static const Material Chrome;
    static const Material Copper;
    static const Material Gold;
    static const Material Silver;
    static const Material BlackPlastic;
    static const Material CyanPlastic;
    static const Material GreenPlastic;
    static const Material RedPlastic;
    static const Material WhitePlastic;
    static const Material YellowPlastic;
    static const Material BlackRubber;
    static const Material CyanRubber;
    static const Material GreenRubber;
    static const Material RedRubber;
    static const Material WhiteRubber;
    static const Material YellowRubber;
};