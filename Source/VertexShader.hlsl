struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};
 
struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

cbuffer Transforms : register(b0)
{
    float4x4 mvp;
};
 
VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
 
    OUT.Position = n mul(float4(IN.Position, 1.0), mvp);;
    OUT.Color = float4(IN.Color, 1.0f);
 
    return OUT;
}