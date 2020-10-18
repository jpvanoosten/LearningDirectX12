struct PixelShaderInput
{
    float4 Color : COLOR;
};

float4 main( PixelShaderInput IN ) : SV_Target
{
    // Return gamma corrected result.
    return pow( abs( IN.Color ), 1.0f / 2.2f );
    // return IN.Color;
}