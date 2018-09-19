Texture2DMS<float4> HDRTexture : register( t0 );

float4 main( float4 Position : SV_Position ) : SV_Target0
{
    uint width, height, numSamples;
    HDRTexture.GetDimensions( width, height, numSamples );

    float4 HDR = (float4)0;
    int2 texCoord = ( int2 )Position.xy;
    for ( uint i = 0; i < numSamples; ++i )
    {
        HDR += HDRTexture.Load( texCoord, i );
    }
    HDR /= numSamples;

    // Probably the simplest method possible.
    return saturate( HDR );
}