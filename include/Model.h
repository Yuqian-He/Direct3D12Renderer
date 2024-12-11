// Model.h
#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include "Vertex.h"
#include <string> 

struct Model {
    std::vector<Vertex> vertices;       // 顶点数据
    std::vector<UINT> indices;          // 索引数据

    void LoadModel(const std::string& filePath, std::vector<Vertex>& outVertices, std::vector<UINT>& outIndices);  // 加载模型数据
};