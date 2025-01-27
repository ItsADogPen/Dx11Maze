struct PSInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.color, 1.0f);
}