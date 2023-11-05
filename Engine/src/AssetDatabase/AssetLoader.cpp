#include "AssetLoader.h"
#include "Log.h"
#include "tiny_obj_loader.h"
#include "../mesh.h"

namespace gns
{
    std::shared_ptr<Mesh> AssetLoader::LoadMesh(std::string path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            LOG_ERROR(warn + err);
        }


        std::vector<Vertex> vertices = {};
	    std::vector<uint32_t> indices = {};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                gns::Vertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
				    attrib.texcoords[2 * index.texcoord_index + 0],
				    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
        //fill vectors up here
        return std::make_shared<Mesh>(std::move(vertices), std::move(indices));
    }
}
