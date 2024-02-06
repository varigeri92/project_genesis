#include "CameraSystem.h"
#include "core.h"
#include "Log.h"

#include "genesis.h"
#include "Editor/gui/EntityInspector.h"
#include "Editor/ProjectExplorer/ContentBrowser.h"
#include "ImGui/imgui.h"

#include "Components/ClearColorComponent.h"


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
		std::shared_ptr<Mesh> suzan_mesh = AssetLoader::LoadMesh(R"(Meshes\OtherModels\Rei\Rei.obj)");
		RenderSystem::S_Renderer->UploadMesh(suzan_mesh.get());

		std::shared_ptr<Mesh> groundPlaneMesh = AssetLoader::LoadMesh(R"(Meshes\plane.obj)");
		RenderSystem::S_Renderer->UploadMesh(groundPlaneMesh.get());

		std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("blinphong.vert.spv", "blinphong.frag.spv");
		LOG_INFO(defaultShader->GetGuid());
		RenderSystem::S_Renderer->CreatePipelineForMaterial(defaultShader);
		std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>(defaultShader, "Rei_Material");
		LOG_INFO(defaultMaterial->GetGuid());
		defaultMaterial->m_texture = std::make_shared<Texture>(R"(Textures\lost_empire-RGBA.png)");
		defaultMaterial->m_texture->Apply();
		//std::shared_ptr<Texture> icon = std::make_shared<Texture>(R"(Resources\Icons\icon_file.png)");
		//icon->Apply();

		//Entity Creation:

		Scene* scene = SceneManager::CreateScene("Default Scene created With the manager!");
		scene->sceneData.ambientColor = { 0.299f,0.145f,0.145f,0.150f };
		scene->sceneData.sunlightColor = { 1.f, 1.f, 1.f, 1.f };
		scene->sceneData.sunlightDirection = { 1.f, 1.f, 0.f , 0.f };
		Entity suzan = scene->CreateEntity("Ayanami_Rei", scene);
		suzan.AddComponet<RendererComponent>(suzan_mesh, defaultMaterial);
		suzan.GetComponent<Transform>().matrix = glm::mat4{ 1 };
		//Create Scene


		Entity sceneCamera_entity = SceneManager::GetActiveScene()->CreateEntity("SceneCamera",
			SceneManager::GetActiveScene());
		sceneCamera_entity.AddComponet<ClearColor>();

		Transform& cameraTransform = sceneCamera_entity.GetTransform();
		cameraTransform.position = { 0.f,-1.f,-3.f };
		CameraComponent& sceneCamera = sceneCamera_entity.AddComponet<CameraComponent>(0.01f, 1000.f, 60.f, 1700, 900, cameraTransform);

		CameraSystem* cameraSystem = new CameraSystem(cameraTransform, sceneCamera);
		core::SystemsApi::RegisterSystem(cameraSystem);
		ContentBrowser* contentBrowser = new ContentBrowser();
		EntityInspector* inspector = new EntityInspector();
		SceneHierarchy* sceneHierarchy = new SceneHierarchy();


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

	});
}

