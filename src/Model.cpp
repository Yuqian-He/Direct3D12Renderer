// Model.cpp
#include "Renderer.h"
#include "Vertex.h"
#include "ModelLoader.h"
#include "Model.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <wrl.h>
#include "d3dx12.h"
#include <memory>
#include <string>
#include <iostream>
#include <d3dcompiler.h>
#include <Windows.h>
#include <dinput.h>

// 加载模型数据
void Model::LoadModel(const std::string& filePath, std::vector<Vertex>& outVertices, std::vector<UINT>& outIndices) {
    // 使用 ModelLoader 加载数据到 outVertices 和 outIndices
    if (!ModelLoader::LoadOBJ(filePath, outVertices, outIndices)) {
        std::cerr << "Failed to load model from " << filePath << std::endl;
    }
}
