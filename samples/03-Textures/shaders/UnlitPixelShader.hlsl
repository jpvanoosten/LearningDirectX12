struct PixelShaderInput
{
    float4 PositionVS : POSITION;
    float3 NormalVS   : NORMAL;
    float2 TexCoord   : TEXCOORD;
};

struct Material
{
    float4 Diffuse;
    //------------------------------------ ( 16 bytes )
    float4 Specular;
    //------------------------------------ ( 16 bytes )
    float4 Emissive;
    //------------------------------------ ( 16 bytes )
    float4 Ambient;
    //------------------------------------ ( 16 bytes )
    float4 Reflectance;
    //------------------------------------ ( 16 bytes )
    float Opacity;            // If Opacity < 1, then the material is transparent.
    float SpecularPower;
    float IndexOfRefraction;  // For transparent materials, IOR > 0.
    float BumpIntensity;      // When using bump textures (height maps) we need
                              // to scale the height values so the normals are visible.
    //------------------------------------ ( 16 bytes )
    float AlphaThreshold;     // Pixels with alpha < m_AlphaThreshold will be discarded.
    bool  HasAmbientTexture;
    bool  HasEmissiveTexture;
    bool  HasDiffuseTexture;
    //------------------------------------ ( 16 bytes )
    bool  HasSpecularTexture;
    bool  HasSpecularPowerTexture;
    bool  HasNormalTexture;
    bool  HasBumpTexture;
    //------------------------------------ ( 16 bytes )
    bool  HasOpacityTexture;
    float3 Padding;           // Pad to 16 byte boundary.
    //------------------------------------ ( 16 bytes )
    // Total:                              ( 16 * 9 = 144 bytes )
};

ConstantBuffer<Material> MaterialCB : register( b0, space1 );
Texture2D DiffuseTexture            : register( t2 );

SamplerState LinearRepeatSampler    : register(s0);

float3 LinearToSRGB( float3 x )
{
    // This is exactly the sRGB curve
    //return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;

    // This is cheaper but nearly equivalent
    return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt( abs( x - 0.00228 ) ) - 0.13448 * x + 0.005719;
}

float4 main( PixelShaderInput IN ) : SV_Target
{
    float4 emissive = MaterialCB.Emissive;
    float4 ambient = MaterialCB.Ambient;
    float4 diffuse  = MaterialCB.Diffuse;
    float4 specular = MaterialCB.Specular;
    float4 texColor = DiffuseTexture.Sample( LinearRepeatSampler, IN.TexCoord );

    //return float4( IN.TexCoord, 0, 0 );
    return ( emissive + ambient + diffuse + specular ) * texColor * -IN.NormalVS.z;
}