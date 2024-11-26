#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "Vertex.h"
#include <Windows.h>

// 模型加载器
class ModelLoader {
public:
    static bool LoadOBJ(const std::string& filename,
                        std::vector<Vertex>& vertices,
                        std::vector<UINT>& indices);
};