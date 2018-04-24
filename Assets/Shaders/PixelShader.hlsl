struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD;
};

Texture2D DiffuseTexture            : register(t0);
SamplerState LinearRepeatSampler    : register(s0);

float4 main( PixelShaderInput IN ) : SV_Target
{
    return DiffuseTexture.Sample( LinearRepeatSampler, IN.TexCoord );
}