#include "gnspch.h"
#include "GuiSystem.h"
#include "../Renderer/Rendering.h"
#include "../Window/Window.h"
#include "../Renderer/Utils/VkDebug.h"
#include "../AssetDatabase/AssetLoader.h"
#include "../Gui/ImGui/IconsMaterialDesign.h"
#include "ImGui/ImGuizmo.h"

std::vector<gns::gui::GuiWindow*> gns::gui::GuiSystem::guiWindows = {};
bool gns::gui::GuiSystem::s_showDemo = false;

ImFont* gns::gui::GuiSystem::iconFont_Big = nullptr;
ImFont* gns::gui::GuiSystem::defaultFont = nullptr;
ImFont* gns::gui::GuiSystem::boldFont = nullptr;


gns::gui::GuiSystem::GuiSystem(rendering::Device* device) : m_device(device)
{
	//1: create descriptor pool for IMGUI
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;


	_VK_CHECK(vkCreateDescriptorPool(m_device->m_device, &pool_info, nullptr, &m_device->m_imGuiPool), "Failed to create DescriptorPools for GUI");


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplSDL2_InitForVulkan(Window::getInstance()->sdlWindow);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_device->m_instance;
	init_info.PhysicalDevice = m_device->m_physicalDevice;
	init_info.Device = m_device->m_device;
	init_info.Queue = m_device->m_graphicsQueue;
	init_info.DescriptorPool = m_device->m_imGuiPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = m_device->m_imageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	
	ImGui_ImplVulkan_Init(&init_info, m_device->m_screenPass.renderPass);

	//execute a gpu command to upload imgui font textures
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;         // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();

	LOG_INFO(gns::AssetLoader::GetEngineResourcesPath());
	std::string fontsDirectory = gns::AssetLoader::GetEngineResourcesPath() + R"(Fonts\static\)";
	defaultFont = io.Fonts->AddFontFromFileTTF((fontsDirectory + R"(Montserrat-Regular.ttf)").c_str(),
		14, nullptr, io.Fonts->GetGlyphRangesDefault());
	std::string iconFonts = gns::AssetLoader::GetEngineResourcesPath() + R"(Fonts\MaterialIcons-Regular.ttf)";
	static ImWchar ranges[] = {ICON_MIN_MD, ICON_MAX_16_MD};
	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 0;
	config.GlyphOffset = { 0, 5 };

	io.Fonts->AddFontFromFileTTF(iconFonts.c_str(), 18, &config, ranges);

	boldFont = io.Fonts->AddFontFromFileTTF((fontsDirectory + R"(Montserrat-SemiBold.ttf)").c_str(),
		14, nullptr, io.Fonts->GetGlyphRangesDefault());
	io.Fonts->AddFontFromFileTTF(iconFonts.c_str(), 18, &config, ranges);


	iconFont_Big = io.Fonts->AddFontFromFileTTF((fontsDirectory + R"(Montserrat-Regular.ttf)").c_str(),
		35, nullptr, io.Fonts->GetGlyphRangesDefault());
	config.GlyphOffset = { 0, 5 };
	io.Fonts->AddFontFromFileTTF(iconFonts.c_str(), 30, &config, ranges);



	io.Fonts->Build();
	m_device->ImmediateSubmit([&](VkCommandBuffer cmd) {
		ImGui_ImplVulkan_CreateFontsTexture();
		});

	s_showDemo = false;
}

void gns::gui::GuiSystem::BeginGUI() const
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(Window::getInstance()->sdlWindow);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void gns::gui::GuiSystem::UpdateGui()
{
	if(s_showDemo)
		ImGui::ShowDemoWindow();

	for (auto window : guiWindows)
	{
		window->DrawWindow();
	}
}

void gns::gui::GuiSystem::PresentGui(const VkCommandBuffer cmd)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	ImGui::EndFrame();
}


void gns::gui::GuiSystem::DisposeGUI() const
{
	vkDeviceWaitIdle(m_device->m_device);
	for (size_t i = 0; i<guiWindows.size(); i++ )
	{
		delete guiWindows[i];
	}
		
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(m_device->m_device, m_device->m_imGuiPool, nullptr);
}

void gns::gui::GuiSystem::ToggleDemoWindow()
{
	s_showDemo = !s_showDemo;
}
