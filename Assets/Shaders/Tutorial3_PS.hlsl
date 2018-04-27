struct PixelShaderInput
{
    float4 PositionVS : POSITION;
    float3 NormalVS   : NORMAL;
    float2 TexCoord   : TEXCOORD;
};

struct Material
{
    float4 Emissive;
    //----------------------------------- (16 byte boundary)
    float4 Ambient;
    //----------------------------------- (16 byte boundary)
    float4 Diffuse;
    //----------------------------------- (16 byte boundary)
    float4 Specular;
    //----------------------------------- (16 byte boundary)
    float  SpecularPower;
    float3 Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 5 = 80 bytes
};

ConstantBuffer<Material> MaterialCB : register(b0, space1);
Texture2D DiffuseTexture            : register(t0);
SamplerState LinearRepeatSampler    : register(s0);

float4 main( PixelShaderInput IN ) : SV_Target
{
    float3 normal = normalize( IN.NormalVS );

    float4 emissive = MaterialCB.Emissive;
    float4 ambient = MaterialCB.Ambient;
    float4 diffuse = MaterialCB.Diffuse * -normal.z;
    float4 specular = { 0, 0, 0, 1 }; // MaterialCB.Specular; // Doesn't make sense without lights.
    float4 texColor = DiffuseTexture.Sample( LinearRepeatSampler, IN.TexCoord );

    return ( emissive + ambient + diffuse + specular ) * texColor;
}