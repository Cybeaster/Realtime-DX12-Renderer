

struct PixelShaderInput
{
    float4 Color : COLOR;
};

float4 main(PixelShaderInput Input) : SV_TARGET
{
    return Input.Color;
}
