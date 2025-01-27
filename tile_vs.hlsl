struct VSInput
{
    float3 pos : POSITION;
    float3 col : COLOR;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};


VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.pos = float4(input.pos, 1.0);
    output.color = input.col;
    
    return output;
}