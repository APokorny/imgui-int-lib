#pragma once
#include <string>
#include <limits>
#include <functional>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include "imgui/renderer.h"
#include "vk_mem_alloc.h"
#include "imgui_impl_vulkan.h"

struct ImDrawData;

namespace imgui
{
namespace vulkan
{
struct Renderer : imgui::Renderer
{
   public:
    Renderer(std::function<VkSurfaceKHR(VkInstance)> const& fun, int initial_width, int initial_height);
    void setup_imgui() override;
    void acquire_cached_resources() override;
    void render_imgui_data(ImDrawData& draw_data) override;
    void pre_frame() override;
    void finish_frame() override;
    void resize(size_t w, size_t h) override;
    ~Renderer();

   private:
    vkb::Instance            instance;
    VkAllocationCallbacks*   allocator = NULL;
    vkb::PhysicalDevice      chosen_GPU;
    vkb::Device              device;
    VkQueue                  graphics_queue;
    uint32_t                 graphics_queue_family{std::numeric_limits<uint32_t>::max()};
    VkDebugReportCallbackEXT debug_report    = VK_NULL_HANDLE;
    VkPipelineCache          pipeline_cache  = VK_NULL_HANDLE;
    VkDescriptorPool         descriptor_pool = VK_NULL_HANDLE;

    ImGui_ImplVulkanH_Window main_window_data;
    uint32_t                 MinImageCount    = 2;
    bool                     SwapChainRebuild = false;
    void                     setup_vulkan();
    void                     setup_vulkan_window(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
    void                     cleanup_vulkan();
};

}  // namespace vulkan
}  // namespace imgui