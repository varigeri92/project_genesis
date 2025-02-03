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
#include <yaml-cpp/yaml.h>

#include "AssetDatabase.h"
#include "../Renderer/Material.h"
#include "../Utils/FileSystem/FileSystem.h"

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
				ProcessImportedScene(scene->mMeshes[i], meshData);
                mesh->m_subMeshes.push_back(meshData);
            }
        }
        return mesh;
    }

    rendering::Material* AssetLoader::LoadMaterialFromFile(gns::core::guid guid, std::string path)
    {

        YAML::Node root = YAML::LoadFile(GetAssetsPath()+path);
    	
        rendering::Shader* defaultShader = Object::Create<rendering::Shader>(
            root["V_shader"].as<std::string>(), 
            root["F_shader"].as<std::string>()
        );

        const std::shared_ptr<rendering::Texture> defaultTexture = std::make_shared<rendering::Texture>(R"(Textures\uv_color_Grid.png)");
        
        rendering::Material* material = Object::Create<rendering::Material>(guid,defaultShader, gns::fileSystem::FileSystem::GetFileName(path));
        material->SetTexture(defaultTexture, 0);
        const YAML::Node& attributes = root["attributes"];
        for (std::size_t i = 0; i < attributes.size(); i++)
        {
            int type = attributes[i]["type"].as<int>();
            if(type == 0)
            {
                std::string name = attributes[i]["name"].as<std::string>();
                float value = attributes[i]["value"].as<float>();
            	material->SetFragmentShaderAttribute(name, value);
            }
        	else
            {
	            glm::vec4 vec(0.5, 0.3, 0.2, 1);
	            material->SetFragmentShaderAttribute(attributes[i]["name"].as<std::string>(), vec);
            }
        }
	    return material;
    }


    void AssetLoader::ProcessImportedScene(const void* _mesh, MeshData* outMesh)
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

	    switch (metaData.assetType)
	    {

        case AssetType::null_Asset:
            break;
	    case AssetType::mesh:
		    {
	        if (metaData.state == AssetState::loaded)
	        {
	            LOG_INFO("Asset: " << metaData.guid << " is already loaded!");
	            return Object::Get<Mesh>(metaData.guid);
	        }
            metaData.state = AssetState::loaded;
            Mesh* mesh = LoadMeshFile(metaData.guid, metaData.sourcePath);
            return mesh;
		    }
        case AssetType::material:
		    {

            if (metaData.state == AssetState::loaded)
            {
                LOG_INFO("Asset: " << metaData.guid << " is already loaded!");
                return Object::Get<Material>(metaData.guid);
            }
            metaData.state = AssetState::loaded;
            Material* mat = LoadMaterialFromFile(metaData.guid, metaData.sourcePath);
            return mat;
		    }
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

