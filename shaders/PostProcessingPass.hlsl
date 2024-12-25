Texture2D sceneTexture : register(t0); // 场景渲染目标
SamplerState samplerState : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET {
    // 直接采样场景纹理进行简单的后处理
    float4 color = sceneTexture.Sample(samplerState, input.texcoord);
    // 应用伽马校正
    color.rgb = pow(color.rgb, 1.0 / 2.2);
    return color;
}
