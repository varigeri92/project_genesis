#include "CameraSystem.h"
#include "core.h"
#include "Log.h"

#include "genesis.h"
#include "Editor/gui/EntityInspector.h"
#include "Editor/ProjectExplorer/ContentBrowser.h"
#include "ImGui/imgui.h"



using namespace gns;
using namespace gns::gui;
using namespace gns::editor;
using namespace gns::rendering;

int main()
{
    LOG_INFO("HELLO WORLD");
	gns::Application* app = CreateApplication(ASSETS_PATH);
	app->Start([app]()
	{
		Scene* scene = SceneManager::CreateScene("Default Scene created With the manager!");
		scene->sceneData.ambientColor = { 1.f,1.f,1.f,0.050f };
		scene->sceneData.sunlightColor = { 1.f, 1.f, 1.f, 1.f };
		scene->sceneData.sunlightDirection = { 1.f, 1.f, 0.f , 0.f };

		std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("blinphong.vert.spv", "blinphong.frag.spv");
		RenderSystem::S_Renderer->CreatePipelineForMaterial(defaultShader);
		//uv_color_Grid.png

		/*
		std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>(defaultShader, "DefaultCheckMaterial");
		defaultMaterial->m_texture = std::make_shared<Texture>(R"(Textures\uv_color_Grid.png)");
		defaultMaterial->m_texture->Apply();
		 */

		std::shared_ptr<Material> bodyMaterial = std::make_shared<Material>(defaultShader, "Rei_bodyMaterial");
		//bodyMaterial->m_texture = std::make_shared<Texture>(R"(Textures\uv_color_Grid.png)");
		bodyMaterial->m_texture = std::make_shared<Texture>(R"(Meshes\OtherModels\Rei\Textures\Rei_3_LP_Body_low_BaseColor.png)");
		bodyMaterial->m_texture->Apply();

		std::shared_ptr<Material> hairMaterial = std::make_shared<Material>(defaultShader, "Rei_hairMaterial");
		//hairMaterial->m_texture = std::make_shared<Texture>(R"(Textures\uv_color_Grid.png)");
		hairMaterial->m_texture = std::make_shared<Texture>(R"(Meshes\OtherModels\Rei\Textures\Rei_3_LP_Hair_low_BaseColor.png)");
		hairMaterial->m_texture->Apply();


		std::shared_ptr<Material> headMaterial = std::make_shared<Material>(defaultShader, "Rei_headMaterial");
		//headMaterial->m_texture = std::make_shared<Texture>(R"(Textures\uv_color_Grid.png)");
		headMaterial->m_texture = std::make_shared<Texture>(R"(Meshes\OtherModels\Rei\Textures\Rei_3_LP_Head_low_BaseColor.png)");
		headMaterial->m_texture->Apply();


		std::shared_ptr<Material> bondageMaterial = std::make_shared<Material>(defaultShader, "Rei_bondageMaterial");
		//bondageMaterial->m_texture = std::make_shared<Texture>(R"(Textures\uv_color_Grid.png)");
		bondageMaterial->m_texture = std::make_shared<Texture>(R"(Meshes\OtherModels\Rei\Textures\Rei_Bandage_2_LP_Bandage_BaseColor.png)");
		bondageMaterial->m_texture->Apply();



		auto rei = AssetLoader::LoadMeshFile(R"(Meshes\OtherModels\Rei\Rei.obj)");

		for (const std::shared_ptr<Mesh>& mesh : rei)
		{
			RenderSystem::S_Renderer->UploadMesh(mesh.get());
			Entity reiEntity = scene->CreateEntity(mesh->name, scene);
			RendererComponent& rc = reiEntity.AddComponet<RendererComponent>(mesh, bodyMaterial);
			if (mesh->name == "Face")
				rc.material = headMaterial;
			if (mesh->name == "Bandage")
				rc.material = bondageMaterial;
			if (mesh->name == "Hair")
				rc.material = hairMaterial;
			/*
			*/
			reiEntity.GetComponent<Transform>().matrix = glm::mat4{ 1 };
		}


		//Create Scene
		Entity sceneCamera_entity = SceneManager::GetActiveScene()->CreateEntity("SceneCamera",
			SceneManager::GetActiveScene());

		Transform& cameraTransform = sceneCamera_entity.GetTransform();
		cameraTransform.position = { 0.f,-1.f,-3.f };
		CameraComponent& sceneCamera = sceneCamera_entity.AddComponet<CameraComponent>(0.01f, 1000.f, 60.f, 1700, 900, cameraTransform);

		core::SystemsApi::RegisterSystem<CameraSystem>(cameraTransform, sceneCamera);

		GUI::RegisterWindow<ContentBrowser>();
		GUI::RegisterWindow<EntityInspector>();
		GUI::RegisterWindow<SceneHierarchy>();

		auto* camSystem = core::SystemsApi::GetSystem<CameraSystem>();
		LOG_INFO(camSystem->name);


	});
	app->Run([app]()
	{
		Scene* scene = SceneManager::GetActiveScene();

		ImGui::Begin("Scene Data Editor");

		ImGui::ColorEdit3("Ambient Color", (float*)&scene->sceneData.ambientColor);
		ImGui::DragFloat("Ambient Intensity", &scene->sceneData.ambientColor.w, 0.01f, 0);

		ImGui::ColorEdit3("Light Color", (float*)&scene->sceneData.sunlightColor);
		ImGui::DragFloat("Light Intensity", &scene->sceneData.sunlightColor.w, 0.001f, 0);

		ImGui::DragFloat3("light Direction", (float*)&scene->sceneData.sunlightDirection, 0.001f, -1.0f, 1.0f);

		ImGui::End();

		ImGui::Begin("Systems Inspector");
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
		if (ImGui::BeginTable("table1", 3, flags))
		{
			ImGui::TableSetupColumn("Index");
			ImGui::TableSetupColumn("name");
			ImGui::TableSetupColumn("Status");
			ImGui::TableHeadersRow();

			for (int row = 0; row < core::SystemsApi::Systems.size(); row++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(std::to_string(core::SystemsApi::Systems[row]->index).c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted(core::SystemsApi::Systems[row]->name.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Checkbox("", &core::SystemsApi::Systems[row]->isActive);
			}
			ImGui::EndTable();
		}
		
		ImGui::End();
		ImGui::Begin("Test WindowOpenClose");
		for (int i  =0; i< GUI::guiWindows.size(); i++)
		{
			if (ImGui::Button(GUI::guiWindows[i]->name.c_str()))
				GUI::guiWindows[i]->SetActive(!GUI::guiWindows[i]->m_isActive);
		}

		ImGui::End();

	});
}

