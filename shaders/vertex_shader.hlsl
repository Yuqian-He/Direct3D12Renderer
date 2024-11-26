// 顶点着色器（vertex_shader.hlsl）

// 定义输入结构体，接收从模型加载进来的顶点数据
struct VSInput {
    float3 position : POSITION; // 顶点位置
    float4 color : COLOR;       // 顶点颜色
};

// 定义输出结构体，传递到片段着色器
struct PSInput {
    float4 position : SV_POSITION; // 裁剪空间中的位置
    float4 color : COLOR;          // 颜色
};

// 常量缓冲区，用于传递变换矩阵
cbuffer MatrixBuffer : register(b0) {
    float4x4 worldMatrix;     // 模型矩阵
    float4x4 viewMatrix;      // 视图矩阵
    float4x4 projectionMatrix; // 投影矩阵
};

// 顶点着色器的主函数
PSInput main(VSInput input) {
    PSInput output;

    // 计算模型空间到裁剪空间的坐标
    float4 worldPos = mul(float4(input.position, 1.0f), worldMatrix); // 模型矩阵变换
    float4 viewPos = mul(worldPos, viewMatrix); // 视图矩阵变换
    output.position = mul(viewPos, projectionMatrix); // 投影矩阵变换

    // 传递颜色数据
    output.color = input.color;

    return output;
}
