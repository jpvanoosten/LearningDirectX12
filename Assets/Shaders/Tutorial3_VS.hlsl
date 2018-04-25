struct Matrices
{
    matrix ModelMatrix;
    matrix InverseTransposeModelMatrix;
    matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Matrices> MatricesCB : register(b0);

struct VertexPositionNormalTexture
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionWS : POSITION;
    float3 NormalWS   : NORMAL;
    float2 TexCoord   : TEXCOORD;
    float4 Position   : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTexture IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(MatricesCB.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));
    OUT.PositionWS = mul(MatricesCB.ModelMatrix, float4(IN.Position, 1.0f));
    OUT.NormalWS = mul((float3x3)MatricesCB.InverseTransposeModelMatrix, IN.Normal);
    OUT.TexCoord = IN.TexCoord;

    return OUT;
}