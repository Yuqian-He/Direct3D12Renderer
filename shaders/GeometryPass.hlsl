cbuffer GeometryConstants : register(b0) {
    float4x4 worldViewProj;
};

Texture2D diffuseTexture : register(t0); // 漫反射贴图
SamplerState samplerState : register(s0);

struct VSInput {
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.position = mul(float4(input.position, 1.0), worldViewProj);
    output.texcoord = input.texcoord;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET {
    return diffuseTexture.Sample(samplerState, input.texcoord);
}
