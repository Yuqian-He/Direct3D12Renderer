#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <d3dcompiler.h>

class ShaderCompiler {
public:
    // 编译着色器从文件
    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShaderFromFile(const std::wstring& filePath, 
                                                                  const std::string& entryPoint, 
                                                                  const std::string& target);

    // 加载已编译的着色器
    static Microsoft::WRL::ComPtr<ID3DBlob> LoadCompiledShader(const std::wstring& filePath);
};
