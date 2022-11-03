#include "renderer.h"
#include "imgui.h"

#include "imgui_impl_vulkan.h"
#include <VkBootstrap.h>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <functional>
#include <vector>

namespace ig = imgui::vulkan;
namespace
{
struct Internal_ImGuiVulkan_Data
{
    ImGui_ImplVulkan_InitInfo VulkanInitInfo;
    VkRenderPass              RenderPass;
    VkDeviceSize              BufferMemoryAlignment;
    VkPipelineCreateFlags     PipelineCreateFlags;
    VkDescriptorSetLayout     DescriptorSetLayout;
    VkPipelineLayout          PipelineLayout;
    VkDescriptorSet           DescriptorSet;
    VkPipeline                Pipeline;
    uint32_t                  Subpass;
    VkShaderModule            ShaderModuleVert;
    VkShaderModule            ShaderModuleFrag;

    // Font data
    VkSampler      FontSampler;
    VkDeviceMemory FontMemory;
    VkImage        FontImage;
    VkImageView    FontView;
    VkDeviceMemory UploadBufferMemory;
    VkBuffer       UploadBuffer;
};
}  // namespace

static uint32_t get_memory_type(VkPhysicalDevice const& dev, VkMemoryPropertyFlags properties, uint32_t type_bits)
{
    VkPhysicalDeviceMemoryProperties prop;
    vkGetPhysicalDeviceMemoryProperties(dev, &prop);
    for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i)) return i;
    return 0xFFFFFFFF;  // Unable to find memoryType
}

static void check_vk_result(VkResult err)
{
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
}

void ig::Renderer::cleanup_vulkan()
{
    ImGui_ImplVulkanH_DestroyWindow(instance, device, &main_window_data, allocator);
    vkDestroyDescriptorPool(device, descriptor_pool, allocator);

    vkb::destroy_device(device);
    vkb::destroy_instance(instance);
}

ig::Renderer::Renderer(std::function<VkSurfaceKHR(VkInstance)> const& fun, int initial_width, int initial_height)
{
    vkb::InstanceBuilder builder;
    auto const           debug_layer = true;
    auto                 prev        = builder               //
                    .set_engine_name("Dear, Imgui")          //
                    .set_app_name("ui application")          //
                    .request_validation_layers(debug_layer)  //
                    .require_api_version(1, 1, 0);
    // prev.enable_extension("VK_KHR_swapchain");
    if (debug_layer) prev.use_default_debug_messenger();
    auto vkb_inst = prev.build();
    instance      = vkb_inst.value();

    auto                        surface = fun(instance);
    vkb::PhysicalDeviceSelector phys_device_selector(instance);
    auto                        phys_device_return = phys_device_selector.set_surface(surface).select();
    if (!phys_device_return)
    {
        // Handle error
    }
    chosen_GPU = phys_device_return.value();

    vkb::DeviceBuilder device_builder{chosen_GPU};
    auto               dev_ret = device_builder.build();
    device                     = dev_ret.value();

    main_window_data.Surface = surface;
    graphics_queue           = device.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family    = device.get_queue_index(vkb::QueueType::graphics).value();

    uint32_t res;
    vkGetPhysicalDeviceSurfaceSupportKHR(chosen_GPU, graphics_queue_family, main_window_data.Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat        surface_image_format[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM,
                                             VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR surface_color_space    = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    main_window_data.SurfaceFormat               = ImGui_ImplVulkanH_SelectSurfaceFormat(
                      chosen_GPU, main_window_data.Surface, surface_image_format, sizeof(surface_image_format) / sizeof(VkFormat), surface_color_space);

    // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
    main_window_data.PresentMode =
        ImGui_ImplVulkanH_SelectPresentMode(chosen_GPU, main_window_data.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(instance, chosen_GPU, device, &main_window_data, graphics_queue_family, allocator, initial_width,
                                           initial_height, MinImageCount);
}

ig::Renderer::~Renderer()
{
    ImGui_ImplVulkan_Shutdown();
    cleanup_vulkan();
}

void ig::Renderer::render_imgui_data(ImDrawData& draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = main_window_data.FrameSemaphores[main_window_data.SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = main_window_data.FrameSemaphores[main_window_data.SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(device, main_window_data.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE,
                                &main_window_data.FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &main_window_data.Frames[main_window_data.FrameIndex];
    {
        err = vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);  // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info    = {};
        info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass               = main_window_data.RenderPass;
        info.framebuffer              = fd->Framebuffer;
        info.renderArea.extent.width  = main_window_data.Width;
        info.renderArea.extent.height = main_window_data.Height;
        info.clearValueCount          = 1;
        info.pClearValues             = &main_window_data.ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(&draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo         info       = {};
        info.sType                      = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount         = 1;
        info.pWaitSemaphores            = &image_acquired_semaphore;
        info.pWaitDstStageMask          = &wait_stage;
        info.commandBufferCount         = 1;
        info.pCommandBuffers            = &fd->CommandBuffer;
        info.signalSemaphoreCount       = 1;
        info.pSignalSemaphores          = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(graphics_queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

void ig::Renderer::acquire_cached_resources()
{
    VkResult err;
    {
        // Use any command queue
        VkCommandPool   command_pool   = main_window_data.Frames[main_window_data.FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = main_window_data.Frames[main_window_data.FrameIndex].CommandBuffer;

        err = vkResetCommandPool(device, command_pool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        check_vk_result(err);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info       = {};
        end_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers    = &command_buffer;
        err                         = vkEndCommandBuffer(command_buffer);
        check_vk_result(err);
        err = vkQueueSubmit(graphics_queue, 1, &end_info, VK_NULL_HANDLE);
        check_vk_result(err);

        err = vkDeviceWaitIdle(device);
        check_vk_result(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void ig::Renderer::setup_imgui()
{
    VkResult err;
    // Create Descriptor Pool
    {
        VkDescriptorPoolSize       pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
        VkDescriptorPoolCreateInfo pool_info    = {};
        pool_info.sType                         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.poolSizeCount                 = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
        pool_info.maxSets                       = 1000 * pool_info.poolSizeCount;
        pool_info.pPoolSizes                    = pool_sizes;
        err                                     = vkCreateDescriptorPool(device, &pool_info, allocator, &descriptor_pool);
        check_vk_result(err);
    }

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = instance;
    init_info.PhysicalDevice            = chosen_GPU;
    init_info.Device                    = device;
    init_info.QueueFamily               = graphics_queue_family;
    init_info.Queue                     = graphics_queue;
    init_info.PipelineCache             = pipeline_cache;
    init_info.DescriptorPool            = descriptor_pool;
    init_info.Allocator                 = allocator;
    init_info.MinImageCount             = MinImageCount;
    init_info.ImageCount                = main_window_data.ImageCount;
    init_info.CheckVkResultFn           = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, main_window_data.RenderPass);
}

void ig::Renderer::finish_frame()
{
    if (SwapChainRebuild) return;
    VkSemaphore      render_complete_semaphore = main_window_data.FrameSemaphores[main_window_data.SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info                      = {};
    info.sType                                 = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount                    = 1;
    info.pWaitSemaphores                       = &render_complete_semaphore;
    info.swapchainCount                        = 1;
    info.pSwapchains                           = &main_window_data.Swapchain;
    info.pImageIndices                         = &main_window_data.FrameIndex;
    VkResult err                               = vkQueuePresentKHR(graphics_queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    main_window_data.SemaphoreIndex =
        (main_window_data.SemaphoreIndex + 1) % main_window_data.ImageCount;  // Now we can use the next set of semaphores
}
void ig::Renderer::pre_frame()
{
    main_window_data.ClearValue.color.float32[0] = 0;//clear_color.x * clear_color.w;
    main_window_data.ClearValue.color.float32[1] = 0;//clear_color.y * clear_color.w;
    main_window_data.ClearValue.color.float32[2] = 0;//clear_color.z * clear_color.w;
    main_window_data.ClearValue.color.float32[3] = 1.0;//clear_color.w;
}

void ig::Renderer::resize(size_t width, size_t height)
{
    ImGui_ImplVulkan_SetMinImageCount(MinImageCount);
    ImGui_ImplVulkanH_CreateOrResizeWindow(instance, chosen_GPU, device, &main_window_data, graphics_queue_family, allocator, width, height,
                                           MinImageCount);
    main_window_data.FrameIndex = 0;
    SwapChainRebuild            = false;
}

std::unique_ptr<imgui::Texture> ig::Renderer::create_texture(std::size_t width, std::size_t height, bool alpha)
{
    auto*           bd             = reinterpret_cast<Internal_ImGuiVulkan_Data*>(ImGui::GetIO().BackendRendererUserData);
    auto&           iinfo          = bd->VulkanInitInfo;
    VkCommandPool   command_pool   = main_window_data.Frames[main_window_data.FrameIndex].CommandPool;
    VkCommandBuffer command_buffer = main_window_data.Frames[main_window_data.FrameIndex].CommandBuffer;
    VkImage         image;
    VkImageView     image_view;
    VkSampler       image_sampler;
    VkDeviceMemory  image_memory;
    // tODO uploading should happen in a differnet queue and probably separate command buffer... and ...
    VkResult err;

    // Create the Image:
    {
        VkImageCreateInfo info = {};
        info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType         = VK_IMAGE_TYPE_2D;
        info.format            = alpha ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_B8G8R8_UNORM;
        info.extent.width      = width;
        info.extent.height     = height;
        info.extent.depth      = 1;
        info.mipLevels         = 1;
        info.arrayLayers       = 1;
        info.samples           = VK_SAMPLE_COUNT_1_BIT;
        info.tiling            = VK_IMAGE_TILING_OPTIMAL;
        info.usage             = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
        err                    = vkCreateImage(iinfo.Device, &info, iinfo.Allocator, &image);
        check_vk_result(err);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(iinfo.Device, image, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize       = req.size;
        alloc_info.memoryTypeIndex      = get_memory_type(chosen_GPU, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
        err                             = vkAllocateMemory(iinfo.Device, &alloc_info, iinfo.Allocator, &image_memory);
        check_vk_result(err);
        err = vkBindImageMemory(iinfo.Device, image, image_memory, 0);
        check_vk_result(err);
    }

    // Create the Image View:
    {
        VkImageViewCreateInfo info       = {};
        info.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image                       = image;
        info.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
        info.format                      = alpha ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8_UNORM;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        err                              = vkCreateImageView(iinfo.Device, &info, iinfo.Allocator, &image_view);
        check_vk_result(err);
    }

    // Update the Descriptor Set:
    {
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler               = image_sampler;
        desc_image[0].imageView             = image_view;
        desc_image[0].imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet write_desc[1]  = {};
        write_desc[0].sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet                = bd->DescriptorSet;
        write_desc[0].descriptorCount       = 1;
        write_desc[0].descriptorType        = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo            = desc_image;
        vkUpdateDescriptorSets(iinfo.Device, 1, write_desc, 0, NULL);
    }

    return std::make_unique<ig::Texture>(width, height, alpha, image, image_view, image_sampler, image_memory, command_pool, command_buffer,
                                         graphics_queue);
}

void ig::Texture::upload(unsigned char* buffer, std::size_t stride, std::size_t height, color_layout pixel_layout)
{
    auto*          bd    = reinterpret_cast<Internal_ImGuiVulkan_Data*>(ImGui::GetIO().BackendRendererUserData);
    auto&          iinfo = bd->VulkanInitInfo;
    VkResult       err;
    VkBuffer       upload_buffer;
    VkDeviceMemory upload_buffer_memory;
    auto           upload_size = height * stride;
    // Create the Upload Buffer:
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size               = upload_size;
        buffer_info.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
        err                            = vkCreateBuffer(iinfo.Device, &buffer_info, iinfo.Allocator, &upload_buffer);
        check_vk_result(err);
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(iinfo.Device, upload_buffer, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize       = req.size;
        alloc_info.memoryTypeIndex      = get_memory_type(iinfo.PhysicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
        err                             = vkAllocateMemory(iinfo.Device, &alloc_info, iinfo.Allocator, &upload_buffer_memory);
        check_vk_result(err);
        err = vkBindBufferMemory(iinfo.Device, upload_buffer, upload_buffer_memory, 0);
        check_vk_result(err);
    }

    // Upload to Buffer:
    {
        char* map = NULL;
        err       = vkMapMemory(iinfo.Device, upload_buffer_memory, 0, upload_size, 0, (void**)(&map));
        check_vk_result(err);
        memcpy(map, buffer, upload_size);
        VkMappedMemoryRange range[1] = {};
        range[0].sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory              = upload_buffer_memory;
        range[0].size                = upload_size;
        err                          = vkFlushMappedMemoryRanges(iinfo.Device, 1, range);
        check_vk_result(err);
        vkUnmapMemory(iinfo.Device, upload_buffer_memory);
    }

    err = vkResetCommandPool(iinfo.Device, command_pool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);
    check_vk_result(err);

    {
        VkImageMemoryBarrier copy_barrier[1]        = {};
        copy_barrier[0].sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier[0].dstAccessMask               = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier[0].oldLayout                   = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier[0].newLayout                   = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier[0].srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].image                       = image;
        copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier[0].subresourceRange.levelCount = 1;
        copy_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                             copy_barrier);

        VkBufferImageCopy region           = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width           = width;
        region.imageExtent.height          = height;
        region.imageExtent.depth           = 1;
        vkCmdCopyBufferToImage(command_buffer, upload_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier use_barrier[1]        = {};
        use_barrier[0].sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier[0].srcAccessMask               = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier[0].dstAccessMask               = VK_ACCESS_SHADER_READ_BIT;
        use_barrier[0].oldLayout                   = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier[0].newLayout                   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier[0].srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].image                       = image;
        use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier[0].subresourceRange.levelCount = 1;
        use_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
                             use_barrier);
    }
    VkSubmitInfo end_info       = {};
    end_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers    = &command_buffer;
    err                         = vkEndCommandBuffer(command_buffer);
    check_vk_result(err);
    err = vkQueueSubmit(upload_queue, 1, &end_info, VK_NULL_HANDLE);
    check_vk_result(err);

    err = vkDeviceWaitIdle(iinfo.Device);
    check_vk_result(err);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    vkFreeMemory(iinfo.Device, upload_buffer_memory, iinfo.Allocator);
    vkDestroyBuffer(iinfo.Device, upload_buffer, iinfo.Allocator);
}
ig::Texture::operator ImTextureID() const { return reinterpret_cast<ImTextureID>(reinterpret_cast<intptr_t>(image)); }
ig::Texture::~Texture()
{
    auto* bd    = reinterpret_cast<Internal_ImGuiVulkan_Data*>(ImGui::GetIO().BackendRendererUserData);
    auto& iinfo = bd->VulkanInitInfo;
    vkDestroySampler(iinfo.Device, image_sampler, iinfo.Allocator);
    vkDestroyImageView(iinfo.Device, image_view, iinfo.Allocator);
    vkFreeMemory(iinfo.Device, image_memory, iinfo.Allocator);
    vkDestroyImage(iinfo.Device, image, iinfo.Allocator);
}
