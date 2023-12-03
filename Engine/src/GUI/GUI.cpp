#include "GUI.h"
#include <vulkan/vulkan.h>
#include "../Rendering/Device.h"
#include "vklog.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl2.h"
#include "ImGui/imgui_impl_vulkan.h"
#include "../Window/Window.h"

gns::GUI::GUI(rendering::Device* device, Window* window) :m_device(device), m_window(window)
{
	InitializeGUI();
}

void gns::GUI::InitializeGUI()
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
	ImGui_ImplSDL2_InitForVulkan(m_window->sdlWindow);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_device->m_instance;
	init_info.PhysicalDevice = m_device->m_physicalDevice;
	init_info.Device = m_device->m_device;
	init_info.Queue = m_device->m_graphicsQueue;
	init_info.DescriptorPool = m_device->m_imGuiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_device->m_renderPass);

	//execute a gpu command to upload imgui font textures
	m_device->ImmediateSubmit([&](VkCommandBuffer cmd) {
		ImGui_ImplVulkan_CreateFontsTexture();
		});
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;         // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();
}

void gns::GUI::BeginGUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_window->sdlWindow);
	ImGui::NewFrame();

}

void gns::GUI::EndGUI()
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_device->GetCurrentFrame()._mainCommandBuffer);
	ImGui::EndFrame();
}

void gns::GUI::DrawGUI()
{
	ImGui::ShowDemoWindow();
}

void gns::GUI::DisposeGUI()
{
	vkDeviceWaitIdle(m_device->m_device);
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(m_device->m_device, m_device->m_imGuiPool, nullptr);
}
