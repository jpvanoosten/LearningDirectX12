
#include <DirectXMath.h>

struct Material
{
    Material()
        : Emissive( 0.0f, 0.0f, 0.0f, 1.0f )
        , Ambient( 0.1f, 0.1f, 0.1f, 1.0f )
        , Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
        , Specular(1.0f, 1.0f, 1.0f, 1.0f)
        , SpecularPower(128.0f) 
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
};