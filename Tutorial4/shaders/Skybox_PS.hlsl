struct PixelShaderInput
{
    // Skybox texture coordinate
    float3 TexCoord : TEXCOORD;
};

TextureCube<float4> SkyboxTexture : register(t0);
SamplerState LinearClampSampler : register(s0);

float4 main(PixelShaderInput IN) : SV_Target
{
    return SkyboxTexture.Sample(LinearClampSampler, IN.TexCoord);
}