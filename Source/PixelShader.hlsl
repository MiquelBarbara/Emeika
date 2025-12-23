#include "CBuffers.hlsli"

Texture2D colourTex : register(t0);
SamplerState colourSampler : register(s0);
 
float4 main(float3 worldPos : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD) : SV_TARGET
{
    float3 Cd = hasDiffuseTex ? colourTex.Sample(colourSampler, texCoord).rgb * diffuseColour.rgb : diffuseColour.rgb;
    float3 N = normalize(normal);
    float3 R = reflect(lightDirection, N);
    float3 V = normalize(view - worldPos);
    float3 specular = pow(saturate(dot(V,R)), shininess);
    float3 diffuse = saturate(- dot(lightDirection, N));

    float3 phongColour = Cd * Kd * diffuse + lightColor * ambientColor * Cd + Ks * lightColor * specular;
    return float4(phongColour, 1.0f );
}