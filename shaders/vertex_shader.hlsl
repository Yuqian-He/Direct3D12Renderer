// 顶点着色器（vertex_shader.hlsl）

// 定义输入结构体，接收从模型加载进来的顶点数据
struct VSInput {
    float3 position : POSITION; // 顶点位置
    float4 color : COLOR;       // 顶点颜色
    float3 normal : NORMAL;     // 顶点法线
};

// 定义输出结构体，传递到片段着色器
struct PSInput {
    float4 position : SV_POSITION; // 裁剪空间中的位置
    float3 worldPos : TEXCOORD0;
    float4 color : COLOR;          // 颜色
    float3 normal : TEXCOORD1;     // 传递到片段着色器的法线
    float4 ShadowCoord : TEXCOORD2; // 光源空间坐标，传递给片段着色器
    float depth : TEXCOORD3; 
};

// 常量缓冲区，用于传递变换矩阵
cbuffer MatrixBuffer : register(b0) {
    float4x4 worldMatrix;     // 模型矩阵
    float4x4 viewMatrix;      // 视图矩阵
    float4x4 projectionMatrix; // 投影矩阵
};

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

// 顶点着色器的主函数
PSInput VSMain(VSInput input) {
    PSInput output;

    // 计算模型空间到裁剪空间的坐标
    float4 worldPos = mul(float4(input.position, 1.0f), worldMatrix); // 模型矩阵变换
    float4 viewPos = mul(worldPos, viewMatrix); // 视图矩阵变换
    output.position = mul(viewPos, projectionMatrix); // 投影矩阵变换

    // 传递颜色数据
    output.color = input.color;
    output.worldPos = worldPos.xyz;

    // 将法线从模型空间转换到世界空间，并进行适当的逆转置变换
    // 使用逆转置矩阵变换法线
    output.normal = mul(input.normal, (float3x3)worldMatrix); // 法线变换到世界空间
    float4x4 lightMVP = lightProjectionMatrix * lightViewMatrix * worldMatrix;
    output.ShadowCoord = mul(float4(input.position, 1.f), lightMVP);
    output.depth = output.position.z / output.position.w;

    return output; 
}

