#include "AssetLoader.h"
#include <fstream>
#include "Log.h"
#include "tiny_obj_loader.h"
#include <glm/glm.hpp>
#include "../Rendering/DataObjects/Mesh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../Rendering/DataObjects/Texture.h"
#include "../Rendering/Helpers/Buffer.h"

using namespace gns::rendering;
namespace gns
{
    std::shared_ptr<Mesh> AssetLoader::LoadMesh(std::string path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,   (AssetsPath+path).c_str()))
        {
            LOG_ERROR(warn + err);
        }

        auto mesh = std::make_shared<Mesh>();
	    std::vector<uint32_t> indices = {};
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

                //hardcode loading to triangles
                int fv = 3;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    //vertex position
                    tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                    //vertex normal
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

                    //copy it into our vertex
                    Vertex new_vert;
                    new_vert.position.x = vx;
                    new_vert.position.y = vy;
                    new_vert.position.z = vz;

                    new_vert.normal.x = nx;
                    new_vert.normal.y = ny;
                    new_vert.normal.z = nz;

                    //we are setting the vertex color as the vertex normal. This is just for display purposes
                    new_vert.color = new_vert.normal;

                    tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
                    tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

                    new_vert.uv.x = ux;
                    new_vert.uv.y = 1 - uy;

                    mesh->_vertices.push_back(new_vert);
                }
                index_offset += fv;
            }
        }
        return mesh;
    }

    std::vector<uint32_t> AssetLoader::LoadShader(std::string path)
    {
        std::ifstream file((AssetsPath + path), std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR("Failed To open File: " << path);
            return {};
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
        file.seekg(0);
        file.read((char*)buffer.data(), fileSize);
        file.close();
        return buffer;

    }

    void AssetLoader::LoadTextureData(std::string path, Texture* texture)
    {
        stbi_uc* pixels = stbi_load((AssetsPath + path).c_str(), &texture->width, &texture->height, &texture->chanels, STBI_rgb_alpha);

        if (!pixels) {
            LOG_ERROR("Can't Open Texture:" << path);
        }
        texture->pixels = pixels;
    }
}
