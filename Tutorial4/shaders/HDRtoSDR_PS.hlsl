// Tonemapping methods
#define TM_Linear     0
#define TM_Reinhard   1
#define TM_ReinhardSq 2
#define TM_ACESFilmic 3

struct TonemapParameters
{
    // The method to use to perform tonemapping.
    uint TonemapMethod;
    // Exposure should be expressed as a relative expsure value (-2, -1, 0, +1, +2 )
    float Exposure;

    // The maximum luminance to use for linear tonemapping.
    float MaxLuminance;

    // Reinhard constant. Generlly this is 1.0.
    float K;

    // ACES Filmic parameters
    // See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
    float A; // Shoulder strength
    float B; // Linear strength
    float C; // Linear angle
    float D; // Toe strength
    float E; // Toe Numerator
    float F; // Toe denominator
    // Note E/F = Toe angle.
    float LinearWhite;

    float Gamma;
};


ConstantBuffer<TonemapParameters> TonemapParametersCB : register( b0 );

// Linear Tonemapping
// Just normalizing based on some maximum luminance
float3 Linear( float3 HDR, float max )
{
    float3 SDR = HDR;
    if ( max > 0 )
    {
        SDR = saturate( HDR / max );
    }

    return SDR;
}

// Reinhard tone mapping.
// See: http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf
float3 Reinhard( float3 HDR, float k )
{
    return HDR / ( HDR + k );
}

// Reinhard^2
// See: http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf
float3 ReinhardSqr( float3 HDR, float k )
{
    return pow( Reinhard( HDR, k ), 2 );
}

// ACES Filmic
// See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
float3 ACESFilmic( float3 x, float A, float B, float C, float D, float E, float F )
{
    return ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - ( E / F );
}

Texture2DMS<float4> HDRTexture : register( t0 );

float4 main( float4 Position : SV_Position ) : SV_Target0
{
    // First perform a MSAA resolve.
    uint width, height, numSamples;
    HDRTexture.GetDimensions( width, height, numSamples );

    float3 HDR = (float3)0;
    int2 texCoord = ( int2 )Position.xy;
    for ( uint i = 0; i < numSamples; ++i )
    {
        HDR += HDRTexture.Load( texCoord, i ).rgb;
    }
    HDR /= numSamples;

    // Add exposure to HDR result.
    HDR *= exp2( TonemapParametersCB.Exposure );

    // Perform tonemapping on HDR image.
    float3 SDR = ( float3 )0;
    switch ( TonemapParametersCB.TonemapMethod )
    {
    case TM_Linear:
        SDR = Linear( HDR, TonemapParametersCB.MaxLuminance );
        break;
    case TM_Reinhard:
        SDR = Reinhard( HDR, TonemapParametersCB.K );
        break;
    case TM_ReinhardSq:
        SDR = ReinhardSqr( HDR, TonemapParametersCB.K );
        break;
    case TM_ACESFilmic:
        SDR = ACESFilmic( HDR, TonemapParametersCB.A, TonemapParametersCB.B, TonemapParametersCB.C, TonemapParametersCB.D, TonemapParametersCB.E, TonemapParametersCB.F ) /
              ACESFilmic(TonemapParametersCB.LinearWhite, TonemapParametersCB.A, TonemapParametersCB.B, TonemapParametersCB.C, TonemapParametersCB.D, TonemapParametersCB.E, TonemapParametersCB.F );
        break;
    }

    return float4( pow( abs(SDR), 1.0f / TonemapParametersCB.Gamma), 1 );
}