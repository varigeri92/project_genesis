#include "AssetImporter.h"
#include "Engine.h"
#include <filesystem>
#include <fstream>
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"

namespace gns::editor
{
	const std::string metaFileExtension = ".gnsmeta";
	static std::unordered_map<std::string, AssetType> AssetTypeMap = {
	{"obj", AssetType::mesh},
	{"fbx", AssetType::mesh},
	{"gltf", AssetType::mesh},
	{"glb", AssetType::mesh},

	{"png", AssetType::texture},
	{"jpeg", AssetType::texture},
	{"jpg", AssetType::texture},
	{"gnsmat", AssetType::material}

	};

	bool AssetImporter::ImportAsset(const std::string& assetPath)
	{
		std::string relpath = gns::fileSystem::FileSystem::GetRelativePath(assetPath);
		LOG_INFO(relpath);
		std::ifstream file(assetPath);
		if (!file.good()) return false;

		if(IsImported(assetPath)) return true;
		
		std::string metaPath = gns::fileSystem::FileSystem::AppendExtension(assetPath, metaFileExtension);
		core::guid assetGuid = core::Guid::GetNewGuid();
		AssetType assetType = AssetTypeMap.at(gns::fileSystem::FileSystem::GetFileExtension(assetPath));

		CreateMeta(metaPath, relpath, assetGuid, assetType);

		return true;
	}

	bool AssetImporter::ImportAsset(const std::string& assetPath, AssetMetadata& out_assetMeta)
	{
		std::ifstream file(assetPath);
		if (!file.good()) return false;
		file.close();

		std::string relpath = gns::fileSystem::FileSystem::GetRelativePath(assetPath);
		LOG_INFO(relpath);
		std::string metaPath = "";
		if(gns::fileSystem::FileSystem::HasExtension(assetPath, metaFileExtension))
		{
			metaPath = assetPath;
		}
		else
		{
			metaPath = gns::fileSystem::FileSystem::AppendExtension(assetPath, metaFileExtension);
		}
		if (IsImported(assetPath))
		{
			YAML::Node metaNode = YAML::LoadFile(metaPath);
			out_assetMeta.sourcePath = metaNode["source_path"].as<std::string>();
			out_assetMeta.guid = metaNode["asset_guid"].as<size_t>();
			out_assetMeta.name = metaNode["asset_name"].as<std::string>();
			out_assetMeta.assetType = static_cast<AssetType>(metaNode["asset_type"].as<size_t>());

			return true;
		}
		out_assetMeta.guid = core::Guid::GetNewGuid();
		out_assetMeta.assetType = AssetTypeMap.at(gns::fileSystem::FileSystem::GetFileExtension(assetPath));
		out_assetMeta.sourcePath = relpath;
		out_assetMeta.name = gns::fileSystem::FileSystem::GetFileName(relpath);
		CreateMeta(metaPath, relpath, out_assetMeta.guid, out_assetMeta.assetType);
		AssetDatabase::AddImportedAssetToDatabase(out_assetMeta);
		return true;
	}

	bool AssetImporter::IsImported(const std::string& path) const
	{
		if(gns::fileSystem::FileSystem::HasExtension(path, metaFileExtension))
			return true;
		
		std::ifstream file(gns::fileSystem::FileSystem::AppendExtension(path, metaFileExtension));
		return file.good();
	}

	void AssetImporter::CreateMeta(const std::string& path, const std::string& assetSourcePath, 
		core::guid guid, AssetType assetType)
	{
		YAML::Emitter out;
		out << YAML::BeginMap
		<< "asset_name" << "asssetName"
		<< "source_path" << assetSourcePath
		<< "asset_guid" << guid
		<< "asset_type" << static_cast<int>(assetType)
		<< YAML::EndMap;
		gns::fileSystem::FileSystem::WriteFile(out.c_str(), path);
	}

	YAML::Emitter& AssetImporter::CreateAssetBaseData(YAML::Emitter& out, const std::string& rel_path, const std::string& name,
		core::guid guid, AssetType assetType)
	{
		out << YAML::BeginMap
			<< "asset_name" << name
			<< "source_path" << rel_path
			<< "asset_guid" << guid
			<< "asset_type" << static_cast<int>(assetType)
			<< YAML::EndMap;
		return out;
	}

	YAML::Emitter& AssetImporter::SerializeMaterialValues(YAML::Emitter& out, gns::rendering::Material* material,
		const std::string& attributeName, rendering::Shader::ShaderAttributeType attributeType_id)
	{
		switch (attributeType_id) {
		case rendering::Shader::ShaderAttributeType::Float:
			out << "value" << *(material->GetFragmentShaderAttribute<float>(attributeName));
			break;
		case rendering::Shader::ShaderAttributeType::Float2:
			{
				glm::vec2 value = *(material->GetFragmentShaderAttribute<glm::vec2>(attributeName));
				out << "value" << YAML::Flow << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
			}
			break;
		case rendering::Shader::ShaderAttributeType::Float3:
		{
			glm::vec3 value = *(material->GetFragmentShaderAttribute<glm::vec3>(attributeName));
			out << "value" << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
		}
			break;
		case rendering::Shader::ShaderAttributeType::Float4:
		{
			glm::vec4 value = *(material->GetFragmentShaderAttribute<glm::vec4>(attributeName));
			out << "value" << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
		}
			break;
		case rendering::Shader::ShaderAttributeType::Mat4:
			break;
		case rendering::Shader::ShaderAttributeType::Color:
		{
			glm::vec3 value = *(material->GetFragmentShaderAttribute<glm::vec3>(attributeName));
			out << "value" << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
		}
			break;
		case rendering::Shader::ShaderAttributeType::Color4:
		{
			glm::vec4 value = *(material->GetFragmentShaderAttribute<glm::vec4>(attributeName));
			out << "value" << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
		}
			break;
		case rendering::Shader::ShaderAttributeType::HdrColor4:
		{
			glm::vec4 value = *(material->GetFragmentShaderAttribute<glm::vec4>(attributeName));
			out << "value" << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
		}
			break;
		case rendering::Shader::ShaderAttributeType::Int:
			out << "value" << material->GetFragmentShaderAttribute<uint32_t>(attributeName);
			break;
		case rendering::Shader::ShaderAttributeType::Flag:
			out << "value" << 0;
			break;
		case rendering::Shader::ShaderAttributeType::Texture:
			out << "value" << 0;
			break;
		default: ;
		}
		return out;
	}

	AssetMetadata AssetImporter::CreateAsset(const std::string& directory, const std::string& name, core::guid guid,
	                                         AssetType assetType)
	{
		const std::string absPath = directory + name + AssetDatabase::GetExtensionByType(assetType);
		const std::string relPath = gns::fileSystem::FileSystem::GetRelativePath(absPath);
		YAML::Emitter out;
		CreateAssetBaseData(out, relPath, name, guid, assetType);
		gns::fileSystem::FileSystem::WriteFile(out.c_str(), absPath);

		AssetMetadata assetMetadata;
		if (ImportAsset(absPath, assetMetadata))
		{
			return assetMetadata;
		}
		return assetMetadata;
	}

	AssetMetadata AssetImporter::CreateAssetFromObject(const std::string& directory, const std::string& name,
		core::guid guid, AssetType assetType, Object* gns_object)
	{
		YAML::Emitter out;
		const std::string absPath = directory + name + AssetDatabase::GetExtensionByType(assetType);
		const std::string relPath = gns::fileSystem::FileSystem::GetRelativePath(absPath);
		gns::rendering::Material* material = dynamic_cast<gns::rendering::Material*>(gns_object);

		out << YAML::BeginMap << "V_shader" << material->m_shader->GetVertexShaderPath()
		<< "F_shader" << material->m_shader->GetFragmentShaderPath()
		<< "attributes" << YAML::BeginSeq;
		for (uint32_t i = 0; i< material->m_shader->m_fragmentShaderAttributes.size(); i++)
		{
			rendering::Shader::AttributeInfo attributeInfo = material->m_shader->m_fragmentShaderAttributes[i];
			glm::vec4 v4 = *(material->GetFragmentShaderAttribute<glm::vec4>(attributeInfo.attributeName));
			out << YAML::BeginMap << "name" << attributeInfo.attributeName
				<< "set" << attributeInfo.set
				<< "type" << static_cast<int>(attributeInfo.type)
				<< "binding" << attributeInfo.binding
				<< "offset" << attributeInfo.offset;
			SerializeMaterialValues(out, material, attributeInfo.attributeName, attributeInfo.type);
			out << YAML::EndMap;
				
		}
		out << YAML::EndSeq;
		gns::fileSystem::FileSystem::WriteFile(out.c_str(), absPath);

		AssetMetadata assetMetadata;
		if (ImportAsset(absPath, assetMetadata))
		{
			return assetMetadata;
		}
		return assetMetadata;
	}
}
