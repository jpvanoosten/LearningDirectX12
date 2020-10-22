struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput main( uint VertexID : SV_VertexID )
{
    VertexShaderOutput OUT;
    // Source: https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/ScreenQuadVS.hlsl
    // Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    OUT.TexCoord = float2( uint2( VertexID, VertexID << 1 ) & 2 );
    OUT.Position = float4( lerp( float2( -1, 1 ), float2( 1, -1 ), OUT.TexCoord ), 0, 1 );

    return OUT;
}