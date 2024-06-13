#include <gnspch.h>
#include "AssetLoader.h"
#include <string>
#include <filesystem>
#include <fstream>
#include "Log.h"
#include <glm/glm.hpp>
#include "../Renderer/MeshData.h"
#include "../Renderer/Utils/Buffer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Loaders/stb_image.h"
#include "../Renderer/Texture.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "AssetDatabase.h"

inline std::string GetFileName(const std::string& filePath) {
    std::filesystem::path path(filePath);
    std::string fileName = path.stem().string();
    return fileName;
}

using namespace gns::rendering;

namespace gns
{
    std::string AssetLoader::AssetsPath = R"(C:\)";
    std::string AssetLoader::ProjectPath = R"(C:\)";
    std::string AssetLoader::ShadersPath = R"(C:\)";
    std::string AssetLoader::ResourcesPath = R"(C:\)";

    Mesh* AssetLoader::LoadMeshFile(gns::core::guid guid, std::string path, bool isFallbackPath)
    {
        Mesh* mesh = Object::Create<Mesh>(guid);
        mesh->m_name = GetFileName(path);
        Assimp::Importer importer;
        const unsigned flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
        const unsigned flags2 = aiProcessPreset_TargetRealtime_Fast;
        const unsigned flags3 = 0;
        LOG_INFO("Load Mesh from: " << AssetsPath + path);
        const aiScene* scene = importer.ReadFile((AssetsPath + path), flags);

        // If the import failed, report it
        if (nullptr == scene) {
            LOG_ERROR(importer.GetErrorString());
            LOG_WARNING("Cannot Load MeshData: " << path << " Falling back to Suzan!");
            if (!isFallbackPath) {
                mesh = LoadMeshFile(guid,R"(Meshes\Suzan.obj)", true);
            }
            else
            {
                assert(mesh->m_subMeshes.size() == 0, "Failed To load the fallback MeshData!");
            }
            return mesh;
        }
        if (scene->HasMeshes())
        {
            for (size_t i = 0; i < scene->mNumMeshes; i++)
            {
                MeshData* meshData = new MeshData();
				ProcessImpoertedScene(scene->mMeshes[i], meshData);
                mesh->m_subMeshes.push_back(meshData);
            }
        }
        return mesh;
    }


    void AssetLoader::ProcessImpoertedScene(const void* _mesh, MeshData* outMesh)
    {
        const aiMesh* mesh = static_cast<const aiMesh*>(_mesh);
        outMesh->name = mesh->mName.C_Str();
        for (uint32_t v = 0; v < mesh->mNumVertices; v++)
        {
            Vertex vertex{};
            vertex.position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, };
            vertex.normal = { mesh->mNormals[v].x,mesh->mNormals[v].y,mesh->mNormals[v].z, };
            if (mesh->HasVertexColors(0))
            {
                vertex.color = { mesh->mColors[0][v].r,mesh->mColors[0][v].g, mesh->mColors[0][v].b };
            }
            else
            {
                vertex.color = { 1,1,1 };
            }
            if(mesh->HasTextureCoords(0))
            {
				vertex.uv = { mesh->mTextureCoords[0][v].x, 1-mesh->mTextureCoords[0][v].y };
            }
            outMesh->_vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& Face = mesh->mFaces[i];
            for (uint32_t i = 0; i < Face.mNumIndices; i++)
            {
                uint32_t v = Face.mIndices[i];
				outMesh->_indices.push_back(v);
            }
        }

    }

    void* AssetLoader::LoadAssetFromFile_internal(AssetMetadata& metaData)
    {
        if (metaData.state == AssetState::loaded)
        {
            LOG_INFO("Asset: " << metaData.guid << " is already loaded!");
            return Object::Get<Mesh>(metaData.guid);
        }

	    switch (metaData.assetType)
	    {
	    case AssetType::mesh:
            metaData.state = AssetState::loaded;
            Mesh* mesh = LoadMeshFile(metaData.guid, metaData.sourcePath);
            return mesh;
	    }
	    return nullptr;
    }


    std::vector<uint32_t> gns::AssetLoader::LoadShader(std::string path)
    {
        std::string p = (ShadersPath + path);
        std::ifstream file(p, std::ios::ate | std::ios::binary);
        if (!file.is_open()) 
        {
            LOG_ERROR("Failed to open file: " << p);
            assert(true);
            return {};
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
        file.seekg(0);
        file.read((char*)buffer.data(), fileSize);
        file.close();
        return buffer;
    }
    void gns::AssetLoader::LoadTextureData(std::string path, Texture* texture, bool isFallbackPath)
    {
        stbi_uc* pixels = stbi_load((AssetsPath + path).c_str(), &texture->width, &texture->height, &texture->chanels, STBI_rgb_alpha);

        if (!pixels) {
            LOG_ERROR("Can't Open Texture:" << (AssetsPath + path).c_str());
            if (!isFallbackPath) {
                LOG_WARNING("Opening Fallback Texture Instead");
                LoadTextureData(R"(Textures\uv_color_Grid.png)", texture, true);
            }
            else
            {
                assert(true, "Could not open any texture!");
            }
        }
        else
        {
            texture->pixels = pixels;
        }
    }

    void AssetLoader::LoadTextureFromResources(std::string path, rendering::Texture* texture)
    {
        stbi_uc* pixels = stbi_load((ResourcesPath + path).c_str(), &texture->width, &texture->height, &texture->chanels, STBI_rgb_alpha);

        if (!pixels) {
            LOG_ERROR("Can't Open Texture:" << (ResourcesPath + path).c_str());
        }
        else
        {
            texture->pixels = pixels;
        }
    }
}

