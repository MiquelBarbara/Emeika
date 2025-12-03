
struct VertexOutput
{
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

cbuffer Transforms : register(b0)
{
    float4x4 mvp;
};
 
VertexOutput main(float3 position : POSITION, float2 texCoord : TEXCOORD)
{
    VertexOutput output;
 
    output.position = mul(float4(position, 1.0), mvp);
    output.texCoord = texCoord;
 
    return output;
}