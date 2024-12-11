// 片段着色器（pixel_shader.hlsl）

// 输入结构体，从顶点着色器接收数据
struct PSInput {
    float4 position : SV_POSITION; // 从顶点着色器传递来的裁剪空间位置
    float3 worldPos : TEXCOORD0;
    float4 color : COLOR;          // 从顶点着色器传递来的颜色
    float3 normal : TEXCOORD1;     // 从顶点着色器传递来的法线
};

// 光源参数
cbuffer LightPixelBuffer : register(b1) {
    float4 lightPosition;   // 光源位置
    float4 lightColor;      // 光源颜色
    float4 ambientColor;    // 环境光颜色
    float ambientIntensity; // 环境光强度
    float3 viewPosition;    // 观察者位置
    float shininess;        // 高光系数
    float specularIntensity;
    float4x4 lightViewMatrix;     // 光源视角矩阵
    float4x4 lightProjectionMatrix; // 光源投影矩阵
}

// 片段着色器的主函数
float4 PSMain(PSInput input) : SV_TARGET {
    // 计算光照

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

    float4 finalColor = saturate(ambient + diffuse + specular); // 
    //return specular;
    return finalColor;
}
