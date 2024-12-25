#include "ShaderCompiler.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>

Microsoft::WRL::ComPtr<ID3DBlob> ShaderCompiler::CompileShaderFromFile(const std::wstring& filePath, 
                                                                      const std::string& entryPoint, 
                                                                      const std::string& target) {
    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    // 使用 D3DCompileFromFile 来编译着色器文件
    HRESULT hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr, // 可以传递自定义宏定义
        nullptr, // 可以传递自定义的 include 文件
        entryPoint.c_str(), // 入口点函数
        target.c_str(), // 编译目标（例如：vs_5_0, ps_5_0）
        compileFlags, // 编译标志
        0, // 默认标志
        &shaderBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile shader from file.");
    }

    return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderCompiler::LoadCompiledShader(const std::wstring& filePath) {
    // 尝试加载已编译的 .cso 文件
    std::ifstream file(filePath, std::ios::binary | std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open compiled shader file.");
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 读取着色器数据
    std::vector<char> shaderData(size);
    file.read(shaderData.data(), size);

    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    // 创建一个新的 ID3DBlob 实例，传入 size 作为大小参数
    HRESULT hr = D3DCreateBlob(size, &shaderBlob);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create ID3DBlob.");
    }

    // 将数据拷贝到 ID3DBlob 缓存
    memcpy(shaderBlob->GetBufferPointer(), shaderData.data(), size);

    return shaderBlob;
}
