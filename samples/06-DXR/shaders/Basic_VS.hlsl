// clang-format off
struct Matrices
{
    matrix ModelMatrix;
    matrix ModelViewMatrix;
    matrix InverseTransposeModelViewMatrix;
    matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Matrices> MatCB : register( b0 );

struct VertexPositionNormalTangentBitangentTexture
{
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord  : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionVS  : POSITION;
    float3 NormalVS    : NORMAL;
    float3 TangentVS   : TANGENT;
    float3 BitangentVS : BITANGENT;
    float2 TexCoord    : TEXCOORD;
    float4 Position    : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTangentBitangentTexture IN)
{
    VertexShaderOutput OUT;

    OUT.PositionVS  = mul( MatCB.ModelViewMatrix, float4(IN.Position, 1.0f));
    OUT.NormalVS    = mul( (float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Normal );
    OUT.TangentVS   = mul( (float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Tangent );
    OUT.BitangentVS = mul( (float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Bitangent );
    OUT.TexCoord    = IN.TexCoord.xy;
    OUT.Position    = mul( MatCB.ModelViewProjectionMatrix, float4( IN.Position, 1.0f ) );

    return OUT;
}