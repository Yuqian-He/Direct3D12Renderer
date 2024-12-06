#include "ModelLoader.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <Windows.h>
#include <unordered_map> // 引入 unordered_map
#include <string>        // 用于 std::string
#include <vector>        // 用于 std::vector
#include <windows.h>     // 如果 UINT 来自 Windows SDK
#include "Vertex.h"      // 包含 Vertex 的定义

bool ModelLoader::LoadOBJ(const std::string& filepath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Load the OBJ file
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

    // Process shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};

            // Load position
            vertex.Position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // Load normals if available
            if (index.normal_index >= 0) {
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.Normal = { 0.0f, 0.0f, 0.0f }; // Default normal if none provided
            }

            // Load texture coordinates if available
            if (index.texcoord_index >= 0) {
                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            } else {
                vertex.TexCoord = { 0.0f, 0.0f }; // Default UV if none provided
            }

            vertices.push_back(vertex);
            indices.push_back(static_cast<uint32_t>(indices.size()));
        }
    }

    std::cout << "Successfully loaded model: " << filepath << std::endl;
    return true;
}

