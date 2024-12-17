// 片段着色器（pixel_shader.hlsl）

// 输入结构体，从顶点着色器接收数据
struct PSInput {
    float4 position : SV_POSITION; // 从顶点着色器传递来的裁剪空间位置
    float3 worldPos : TEXCOORD0;
    float4 color : COLOR;          // 从顶点着色器传递来的颜色
    float3 normal : TEXCOORD1;     // 从顶点着色器传递来的法线
    float4 ShadowCoord : TEXCOORD2;
    float depth : TEXCOORD3; 
};


// 光源参数
cbuffer LightDataBuffer : register(b1) {
    float4 lightPosition;        // 光源位置
    float4 lightColor;           // 光源颜色
    float4 ambientColor;         // 环境光颜色
    float ambientIntensity;      // 环境光强度
    float3 viewPosition;         // 摄像机位置
    float shininess;             // 高光系数
    float specularIntensity;     // 高光强度
    matrix lightViewMatrix;      // 光源视角矩阵
    matrix lightProjectionMatrix; // 光源投影矩阵
    float4 padding2;             // 对齐
    float4 padding3;             // 确保结构体总大小为16字节的倍数
};

// 阴影贴图
Texture2D<float> ShadowMap : register(t0); // 阴影贴图（深度图）
SamplerState ShadowSampler : register(s0); // 采样器

// 片段着色器的主函数
float4 PSMain(PSInput input) : SV_TARGET {
    //float shadow = (Depth <= input.depth) ? 1.0f : 0.0f;
    //return float4(shadow, shadow, shadow, 1.0f);

    float3 lightDir = normalize(lightPosition.xyz - input.worldPos.xyz); // 计算光源到片段的方向
    float3 norm = normalize(input.normal);
    float diff = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = diff * lightColor; //漫反射光照计算
    float4 ambient = ambientIntensity * ambientColor; // 环境光计算

    // 镜面反射光照计算
    float3 viewDir = normalize(viewPosition - input.worldPos.xyz); // 观察方向
    float3 reflectDir = reflect(-lightDir, norm); // 反射方向
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    float4 specular = specularIntensity * spec * lightColor;
    
    // 计算阴影部分
    // 将世界空间位置转换到光源视角空间
    float4 lightSpacePos = mul(float4(input.worldPos, 1.0f), lightViewMatrix);
    lightSpacePos = mul(lightSpacePos, lightProjectionMatrix);

    float2 shadowUV = input.ShadowCoord.xy / input.ShadowCoord.w; // 透视除法
    shadowUV = (shadowUV + 1.0f) * 0.5f;
    shadowUV.y = 1.0 - shadowUV.y;
    float shadowDepth = ShadowMap.Sample(ShadowSampler, shadowUV).r;
    float isInShadow = lightSpacePos.z > shadowDepth ? 1.0f : 0.0f;
    float4 finalColor = saturate(ambient + diffuse + specular) * (1.0f - isInShadow);

    return float4(ShadowMap.Sample(ShadowSampler, shadowUV).rrr,1.0f);

}
