// 片段着色器（pixel_shader.hlsl）

// 输入结构体，从顶点着色器接收数据
struct PSInput {
    float4 position : SV_POSITION; // 从顶点着色器传递来的裁剪空间位置
    float4 color : COLOR;          // 从顶点着色器传递来的颜色
};

// 片段着色器的主函数
float4 main(PSInput input) : SV_TARGET {
    // 返回顶点传递过来的颜色作为输出
    return input.color;
}
