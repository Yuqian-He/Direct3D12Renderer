cbuffer LightingConstants : register(b0) {
    float3 lightDirection; // 平行光方向
    float3 lightColor;     // 光的颜色
    float3 ambientColor;   // 环境光颜色
};

Texture2D albedoTexture : register(t0);   // 漫反射纹理
Texture2D normalTexture : register(t1);   // 法线纹理
Texture2D depthTexture : register(t2);    // 深度纹理
SamplerState samplerState : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET {
    float3 albedo = albedoTexture.Sample(samplerState, input.texcoord).rgb;
    float3 normal = normalize(normalTexture.Sample(samplerState, input.texcoord).rgb * 2.0 - 1.0);

    float NdotL = max(dot(normal, -lightDirection), 0.0);
    float3 diffuse = lightColor * albedo * NdotL;
    float3 ambient = ambientColor * albedo;

    return float4(diffuse + ambient, 1.0);
}
