#include "ModelLoader.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <Windows.h>
#include <unordered_map> // 引入 unordered_map

bool ModelLoader::LoadOBJ(const std::string& filepath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

    if (!warn.empty()) {
        std::cerr << "Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
    }
    if (!success) {
        std::cerr << "Failed to load OBJ file: " << filepath << std::endl;
        return false;
    }

    // 使用完整的索引 (vertex_index, normal_index, texcoord_index) 作为唯一 Key
    struct VertexKey {
        int vertexIndex;
        int normalIndex;
        int texcoordIndex;

        bool operator==(const VertexKey& other) const {
            return vertexIndex == other.vertexIndex &&
                   normalIndex == other.normalIndex &&
                   texcoordIndex == other.texcoordIndex;
        }
    };

    struct KeyHasher {
        std::size_t operator()(const VertexKey& k) const {
            return ((std::hash<int>()(k.vertexIndex) ^
                    (std::hash<int>()(k.normalIndex) << 1)) >> 1) ^
                    (std::hash<int>()(k.texcoordIndex) << 1);
        }
    };

    std::unordered_map<VertexKey, uint32_t, KeyHasher> uniqueVertexMap; 

    // 遍历所有形状
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};

            // 读取顶点坐标
            vertex.Position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // 读取法线
            if (index.normal_index >= 0) {
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.Normal = { 0.0f, 0.0f, 0.0f };
            }

            // 读取纹理坐标
            if (index.texcoord_index >= 0) {
                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // OpenGL -> DirectX 需要反转 V 轴
                };
            } else {
                vertex.TexCoord = { 0.0f, 0.0f };
            }

            // 构造唯一 Key
            VertexKey key = { index.vertex_index, index.normal_index, index.texcoord_index };

            // 如果这个顶点组合没有存储，则存入
            if (uniqueVertexMap.count(key) == 0) {
                uniqueVertexMap[key] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            // 添加索引
            indices.push_back(uniqueVertexMap[key]);
        }
    }

    std::cout << "Successfully loaded model: " << filepath << std::endl;
    return true;
}
