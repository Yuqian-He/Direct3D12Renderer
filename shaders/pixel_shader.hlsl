// 片段着色器（pixel_shader.hlsl）

// 输入结构体，从顶点着色器接收数据
struct PSInput {
    float4 position : SV_POSITION; // 从顶点着色器传递来的裁剪空间位置
    float4 color : COLOR;          // 从顶点着色器传递来的颜色
    float3 normal : TEXCOORD0;     // 从顶点着色器传递来的法线
};

// 光源参数
cbuffer LightPixelBuffer : register(b1) {
    float4 lightPosition;   // 光源位置
    float4 lightColor;      // 光源颜色
}

// 片段着色器的主函数
float4 PSMain(PSInput input) : SV_TARGET {
    // 计算光照
    float3 lightDir = normalize(lightPosition.xyz - input.position.xyz); // 计算光源到片段的方向
    float diff = max(dot(input.normal, lightDir), 0.0f); // 计算漫反射光照强度

    // 最终颜色 = 颜色 * 光照强度 * 光源颜色
    float4 finalColor = input.color * diff * lightColor;

    return finalColor;
}
