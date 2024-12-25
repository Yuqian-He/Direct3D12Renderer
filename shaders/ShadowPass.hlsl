cbuffer ShadowConstants : register(b0) {
    float4x4 viewMatrix;       // 光源视角矩阵
    float4x4 projectionMatrix; // 光源投影矩阵
};

struct VSInput {
    float3 position : POSITION;
};

struct VSOutput {
    float4 position : SV_POSITION; // 裁剪空间位置
    float depth : TEXCOORD0;       // 深度值
};

VSOutput VSMain(VSInput input) {
    VSOutput output;

    // 组合 View 和 Projection 矩阵
    float4 worldPosition = float4(input.position, 1.0);
    float4 viewPosition = mul(worldPosition, viewMatrix);
    float4 clipSpacePosition = mul(viewPosition, projectionMatrix);
    
    output.position = clipSpacePosition;

    // 归一化深度值，范围从 0 到 1
    output.depth = (clipSpacePosition.z / clipSpacePosition.w) * 0.5 + 0.5;

    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET {
    // 将深度值映射到灰度颜色
    float depth = saturate(input.depth); // 确保深度值在 [0,1] 范围内
    return float4(depth, depth, depth, 1.0); // 使用深度值作为灰度
}
