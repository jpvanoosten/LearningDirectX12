float4 main( uint VertexID : SV_VertexID ) : SV_Position
{
    // Source: https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/ScreenQuadVS.hlsl
    // Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    float2 texCoord = float2( uint2( VertexID, VertexID << 1 ) & 2 );
    float4 position = float4( lerp( float2( -1, 1 ), float2( 1, -1 ), texCoord ), 0, 1 );

    return position;
}