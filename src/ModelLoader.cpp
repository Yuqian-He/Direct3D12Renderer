#include "ModelLoader.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <Windows.h>

bool ModelLoader::LoadOBJ(const std::string& filename,
                          std::vector<Vertex>& vertices,
                          std::vector<UINT>& indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        std::cerr << "Failed to load OBJ file: " << warn << err << std::endl;
        return false;
    }

    std::cout << "Loaded OBJ file successfully!" << std::endl;
    std::cout << "Number of shapes: " << shapes.size() << std::endl;
    std::cout << "Number of vertices: " << attrib.vertices.size() / 3 << std::endl;
    std::cout << "Number of normals: " << attrib.normals.size() / 3 << std::endl;
    std::cout << "Number of texcoords: " << attrib.texcoords.size() / 2 << std::endl;

    int vertexCount = 0;
    int indexCount = 0;

    for (const auto& shape : shapes) {
        std::cout << "Processing shape: " << shape.name << std::endl;
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.Position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0) {
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            // Debug output: print some vertex data
            if (vertexCount < 5) { // Print first 5 vertices for inspection
                std::cout << "Vertex " << vertexCount << " - Position: ("
                          << vertex.Position.x << ", " << vertex.Position.y << ", " << vertex.Position.z
                          << "), Normal: ("
                          << vertex.Normal.x << ", " << vertex.Normal.y << ", " << vertex.Normal.z
                          << "), TexCoord: ("
                          << vertex.TexCoord.x << ", " << vertex.TexCoord.y << ")" << std::endl;
            }

            vertices.push_back(vertex);
            indices.push_back(indexCount);
            indexCount++;
            vertexCount++;
        }
    }

    std::cout << "Total vertices loaded: " << vertexCount << std::endl;
    std::cout << "Total indices loaded: " << indexCount << std::endl;

    return true;
}

