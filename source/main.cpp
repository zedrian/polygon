#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <array>
#include <tuple>
#include <vector>

#include <windows.h>


#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES            // do not declared prototypes, so I can load dynamically!

#include <vulkan/vulkan.h>


using std::cout;
using std::endl;
using std::exception;
using std::find;
using std::runtime_error;
using std::tie;
using std::tuple;
using std::vector;


struct vulkan_context
{

    uint32_t width;
    uint32_t height;

    uint32_t presentQueueIdx;
    VkQueue presentQueue;

    VkInstance instance;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;

    vector<VkImage> presentImages;
    VkImage depthImage;
    VkImageView depthImageView;
    VkFramebuffer* frameBuffers;

    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkDevice device;

    VkCommandBuffer setupCmdBuffer;
    VkCommandBuffer drawCmdBuffer;

    VkRenderPass renderPass;

    VkBuffer vertexInputBuffer;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VkDebugReportCallbackEXT callback;
};

HWND createWindow(HINSTANCE hInstance, const char* window_caption);
void checkAvailableValidationLayers();
void checkAvailableExtensions(const char* const* extensions);
VkImage background_image;


vulkan_context context;


void assert(bool flag, const char* const msg = "")
{
    if (!flag)
    {
        throw runtime_error(msg);
    }
}

void checkVulkanResult(VkResult result, const char* const msg)
{
    assert(result == VK_SUCCESS, msg);
}


PFN_vkCreateInstance vkCreateInstance = NULL;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = NULL;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = NULL;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = NULL;
PFN_vkCreateDevice vkCreateDevice = NULL;
PFN_vkGetDeviceQueue vkGetDeviceQueue = NULL;
PFN_vkCreateCommandPool vkCreateCommandPool = NULL;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = NULL;
PFN_vkCreateFence vkCreateFence = NULL;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = NULL;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = NULL;
PFN_vkEndCommandBuffer vkEndCommandBuffer = NULL;
PFN_vkQueueSubmit vkQueueSubmit = NULL;
PFN_vkWaitForFences vkWaitForFences = NULL;
PFN_vkResetFences vkResetFences = NULL;
PFN_vkResetCommandBuffer vkResetCommandBuffer = NULL;
PFN_vkCreateImageView vkCreateImageView = NULL;
PFN_vkCreateImage vkCreateImage = NULL;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = NULL;
PFN_vkAllocateMemory vkAllocateMemory = NULL;
PFN_vkBindImageMemory vkBindImageMemory = NULL;
PFN_vkCreateRenderPass vkCreateRenderPass = NULL;
PFN_vkCreateFramebuffer vkCreateFramebuffer = NULL;
PFN_vkCreateBuffer vkCreateBuffer = NULL;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = NULL;
PFN_vkMapMemory vkMapMemory = NULL;
PFN_vkUnmapMemory vkUnmapMemory = NULL;
PFN_vkBindBufferMemory vkBindBufferMemory = NULL;
PFN_vkCreateShaderModule vkCreateShaderModule = NULL;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout = NULL;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = NULL;
PFN_vkCreateSemaphore vkCreateSemaphore = NULL;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = NULL;
PFN_vkCmdBindPipeline vkCmdBindPipeline = NULL;
PFN_vkCmdSetViewport vkCmdSetViewport = NULL;
PFN_vkCmdSetScissor vkCmdSetScissor = NULL;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = NULL;
PFN_vkCmdDraw vkCmdDraw = NULL;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass = NULL;
PFN_vkDestroyFence vkDestroyFence = NULL;
PFN_vkDestroySemaphore vkDestroySemaphore = NULL;

PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = NULL;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = NULL;

// Windows platform:
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = NULL;

// Swapchain extension:
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = NULL;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = NULL;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = NULL;
PFN_vkQueuePresentKHR vkQueuePresentKHR = NULL;


struct vertex
{
    float x, y, z, w;
};

void win32_LoadVulkan()
{

    HMODULE vulkan_module = LoadLibrary("vulkan-1.dll");

    assert(vulkan_module, "Failed to load vulkan module.");

    vkCreateInstance = (PFN_vkCreateInstance) GetProcAddress(vulkan_module, "vkCreateInstance");
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) GetProcAddress(vulkan_module,
                                                                                                 "vkEnumerateInstanceLayerProperties");
    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) GetProcAddress(vulkan_module,
                                                                                                         "vkEnumerateInstanceExtensionProperties");
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(vulkan_module, "vkGetInstanceProcAddr");
    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) GetProcAddress(vulkan_module,
                                                                                 "vkEnumeratePhysicalDevices");
    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) GetProcAddress(vulkan_module,
                                                                                       "vkGetPhysicalDeviceProperties");
    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) GetProcAddress(
            vulkan_module, "vkGetPhysicalDeviceQueueFamilyProperties");
    vkCreateDevice = (PFN_vkCreateDevice) GetProcAddress(vulkan_module, "vkCreateDevice");
    vkGetDeviceQueue = (PFN_vkGetDeviceQueue) GetProcAddress(vulkan_module, "vkGetDeviceQueue");
    vkCreateCommandPool = (PFN_vkCreateCommandPool) GetProcAddress(vulkan_module, "vkCreateCommandPool");
    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) GetProcAddress(vulkan_module, "vkAllocateCommandBuffers");
    vkCreateFence = (PFN_vkCreateFence) GetProcAddress(vulkan_module, "vkCreateFence");
    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) GetProcAddress(vulkan_module, "vkBeginCommandBuffer");
    vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) GetProcAddress(vulkan_module, "vkCmdPipelineBarrier");
    vkEndCommandBuffer = (PFN_vkEndCommandBuffer) GetProcAddress(vulkan_module, "vkEndCommandBuffer");
    vkQueueSubmit = (PFN_vkQueueSubmit) GetProcAddress(vulkan_module, "vkQueueSubmit");
    vkWaitForFences = (PFN_vkWaitForFences) GetProcAddress(vulkan_module, "vkWaitForFences");
    vkResetFences = (PFN_vkResetFences) GetProcAddress(vulkan_module, "vkResetFences");
    vkResetCommandBuffer = (PFN_vkResetCommandBuffer) GetProcAddress(vulkan_module, "vkResetCommandBuffer");
    vkCreateImageView = (PFN_vkCreateImageView) GetProcAddress(vulkan_module, "vkCreateImageView");
    vkCreateImage = (PFN_vkCreateImage) GetProcAddress(vulkan_module, "vkCreateImage");
    vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) GetProcAddress(vulkan_module,
                                                                                     "vkGetImageMemoryRequirements");
    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) GetProcAddress(vulkan_module,
                                                                                                   "vkGetPhysicalDeviceMemoryProperties");
    vkAllocateMemory = (PFN_vkAllocateMemory) GetProcAddress(vulkan_module, "vkAllocateMemory");
    vkBindImageMemory = (PFN_vkBindImageMemory) GetProcAddress(vulkan_module, "vkBindImageMemory");
    vkCreateRenderPass = (PFN_vkCreateRenderPass) GetProcAddress(vulkan_module, "vkCreateRenderPass");
    vkCreateFramebuffer = (PFN_vkCreateFramebuffer) GetProcAddress(vulkan_module, "vkCreateFramebuffer");
    vkCreateBuffer = (PFN_vkCreateBuffer) GetProcAddress(vulkan_module, "vkCreateBuffer");
    vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) GetProcAddress(vulkan_module,
                                                                                       "vkGetBufferMemoryRequirements");
    vkMapMemory = (PFN_vkMapMemory) GetProcAddress(vulkan_module, "vkMapMemory");
    vkUnmapMemory = (PFN_vkUnmapMemory) GetProcAddress(vulkan_module, "vkUnmapMemory");
    vkBindBufferMemory = (PFN_vkBindBufferMemory) GetProcAddress(vulkan_module, "vkBindBufferMemory");
    vkCreateShaderModule = (PFN_vkCreateShaderModule) GetProcAddress(vulkan_module, "vkCreateShaderModule");
    vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) GetProcAddress(vulkan_module, "vkCreatePipelineLayout");
    vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) GetProcAddress(vulkan_module,
                                                                               "vkCreateGraphicsPipelines");
    vkCreateSemaphore = (PFN_vkCreateSemaphore) GetProcAddress(vulkan_module, "vkCreateSemaphore");
    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) GetProcAddress(vulkan_module, "vkCmdBeginRenderPass");
    vkCmdBindPipeline = (PFN_vkCmdBindPipeline) GetProcAddress(vulkan_module, "vkCmdBindPipeline");
    vkCmdSetViewport = (PFN_vkCmdSetViewport) GetProcAddress(vulkan_module, "vkCmdSetViewport");
    vkCmdSetScissor = (PFN_vkCmdSetScissor) GetProcAddress(vulkan_module, "vkCmdSetScissor");
    vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) GetProcAddress(vulkan_module, "vkCmdBindVertexBuffers");
    vkCmdDraw = (PFN_vkCmdDraw) GetProcAddress(vulkan_module, "vkCmdDraw");
    vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) GetProcAddress(vulkan_module, "vkCmdEndRenderPass");
    vkDestroyFence = (PFN_vkDestroyFence) GetProcAddress(vulkan_module, "vkDestroyFence");
    vkDestroySemaphore = (PFN_vkDestroySemaphore) GetProcAddress(vulkan_module, "vkDestroySemaphore");

}

void win32_LoadVulkanExtensions(vulkan_context& context)
{

    *(void**) &vkCreateDebugReportCallbackEXT = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                              "vkCreateDebugReportCallbackEXT"));
    *(void**) &vkDestroyDebugReportCallbackEXT = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                               "vkDestroyDebugReportCallbackEXT"));
    *(void**) &vkDebugReportMessageEXT = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                       "vkDebugReportMessageEXT"));

    *(void**) &vkCreateWin32SurfaceKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                       "vkCreateWin32SurfaceKHR"));
    *(void**) &vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                                    "vkGetPhysicalDeviceSurfaceSupportKHR"));
    *(void**) &vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                                    "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    *(void**) &vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(
            context.instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    *(void**) &vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(
            context.instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    *(void**) &vkCreateSwapchainKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                    "vkCreateSwapchainKHR"));
    *(void**) &vkGetSwapchainImagesKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                       "vkGetSwapchainImagesKHR"));
    *(void**) &vkAcquireNextImageKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                     "vkAcquireNextImageKHR"));
    *(void**) &vkQueuePresentKHR = reinterpret_cast<void*>(vkGetInstanceProcAddr(context.instance,
                                                                                 "vkQueuePresentKHR"));
}

void checkAllAvailableExtensions();
void createVulkanApplication(const char* const* layers, const char* const* extensions, const char* application_name);
void setupDebugCallback();
void createWindowSurface(HINSTANCE hInstance, HWND windowHandle);
void findCompatiblePhysicalDevice();
void createVulkanDevice(const char* const* layers);
VkCommandPool createCommandPool();
VkCommandBuffer createCommandBuffer(VkCommandPool pool);
tuple<VkFormat, VkColorSpaceKHR> getColorFormatAndSpace();

void checkDesiredImageCount(uint32_t& image_count,
                            const VkSurfaceCapabilitiesKHR& surface_capabilities);
void printSurfaceCapabilities(const VkSurfaceCapabilitiesKHR& surface_capabilities);
void checkSurfaceResolution(VkExtent2D& surface_resolution);
VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice physical_device,
                                                VkSurfaceKHR surface);
void checkSurfaceTransformFlags(VkSurfaceTransformFlagBitsKHR& transform,
                                VkSurfaceCapabilitiesKHR& capabilities);
VkPresentModeKHR getPresentationMode(VkPhysicalDevice physical_device,
                                     VkSurfaceKHR surface);
VkSwapchainKHR createSwapchain(VkDevice device,
                               VkSurfaceKHR surface,
                               uint32_t desired_image_count,
                               VkFormat color_format,
                               VkColorSpaceKHR color_space,
                               VkExtent2D surface_resolution,
                               VkSurfaceTransformFlagBitsKHR pre_transform,
                               VkPresentModeKHR presentation_mode);
vector<VkImage> getSwapchainImages(VkDevice device,
                                   VkSwapchainKHR swapchain);
void submitCommandBufferToQueue(VkQueue queue,
                                VkCommandBuffer buffer,
                                VkFence submit_fence);

VkImage createDepthImage(VkDevice device, uint32_t width, uint32_t height, VkFormat format);
tuple<VkDeviceMemory, uint32_t> allocateDeviceMemoryForImage(VkDevice device, VkImage image);

void bindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, int offset);
VkImageView createDepthImageView(VkDevice device, VkImage image, VkFormat format);
VkRenderPass createRenderPass(VkDevice device, VkFormat color_format);
VkFramebuffer createFramebuffer(VkDevice device,
                                VkRenderPass pass,
                                uint32_t width,
                                uint32_t height,
                                VkImageView color_image_view,
                                VkImageView depth_image_view);

VkBuffer createVertexInputBuffer(VkDevice device);
VkDeviceMemory allocateDeviceMemoryForBuffer(VkDevice device, VkBuffer buffer, uint32_t memory_type_bits);

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugReportFlagsEXT flags,
                                                     VkDebugReportObjectTypeEXT objectType,
                                                     uint64_t object,
                                                     size_t location,
                                                     int32_t messageCode,
                                                     const char* pLayerPrefix,
                                                     const char* pMessage,
                                                     void* pUserData)
{

    OutputDebugStringA(pLayerPrefix);
    OutputDebugStringA(" ");
    OutputDebugStringA(pMessage);
    OutputDebugStringA("\n");
    return VK_FALSE;
}

void render()
{

    VkSemaphore presentCompleteSemaphore, renderingCompleteSemaphore;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0};
    vkCreateSemaphore(context.device, &semaphoreCreateInfo, NULL, &presentCompleteSemaphore);
    vkCreateSemaphore(context.device, &semaphoreCreateInfo, NULL, &renderingCompleteSemaphore);

    uint32_t nextImageIdx;
    vkAcquireNextImageKHR(context.device, context.swapChain, UINT64_MAX,
                          presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIdx);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(context.drawCmdBuffer, &beginInfo);

    // change image layout from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    VkImageMemoryBarrier layoutTransitionBarrier = {};
    layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    layoutTransitionBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.image = context.presentImages[nextImageIdx];
    VkImageSubresourceRange resourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    layoutTransitionBarrier.subresourceRange = resourceRange;

    vkCmdPipelineBarrier(context.drawCmdBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &layoutTransitionBarrier);

    // activate render pass:
    VkClearValue clearValue[] = {{1.0f, 1.0f, 1.0f, 1.0f},
                                 {1.0,  0.0}};
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = context.renderPass;
    renderPassBeginInfo.framebuffer = context.frameBuffers[nextImageIdx];
    renderPassBeginInfo.renderArea = {0, 0, context.width, context.height};
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValue;
    vkCmdBeginRenderPass(context.drawCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // bind the graphics pipeline to the command buffer. Any vkDraw command afterwards is affeted by this pipeline!
    vkCmdBindPipeline(context.drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline);

    // take care of dynamic state:
    VkViewport viewport = {0, 0, context.width, context.height, 0, 1};
    vkCmdSetViewport(context.drawCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0, 0, context.width, context.height};
    vkCmdSetScissor(context.drawCmdBuffer, 0, 1, &scissor);

    // render the triangle:
    VkDeviceSize offsets = {};
    vkCmdBindVertexBuffers(context.drawCmdBuffer, 0, 1, &context.vertexInputBuffer, &offsets);

    vkCmdDraw(context.drawCmdBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(context.drawCmdBuffer);

    // change layout back to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    VkImageMemoryBarrier prePresentBarrier = {};
    prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    prePresentBarrier.image = context.presentImages[nextImageIdx];
    vkCmdPipelineBarrier(context.drawCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &prePresentBarrier);

    vkEndCommandBuffer(context.drawCmdBuffer);

    // present:
    VkFence renderFence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(context.device, &fenceCreateInfo, NULL, &renderFence);

    VkPipelineStageFlags waitStageMash = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
    submitInfo.pWaitDstStageMask = &waitStageMash;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context.drawCmdBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
    vkQueueSubmit(context.presentQueue, 1, &submitInfo, renderFence);

    vkWaitForFences(context.device, 1, &renderFence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(context.device, renderFence, NULL);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderingCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &context.swapChain;
    presentInfo.pImageIndices = &nextImageIdx;
    presentInfo.pResults = NULL;
    vkQueuePresentKHR(context.presentQueue, &presentInfo);

    vkDestroySemaphore(context.device, presentCompleteSemaphore, NULL);
    vkDestroySemaphore(context.device, renderingCompleteSemaphore, NULL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            render();
            break;

        default:
            break;
    }

    // a pass-through for now. We will return to this callback
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        HWND windowHandle = createWindow(hInstance, "No polygons");

        win32_LoadVulkan();

        checkAvailableValidationLayers();
        const char* layers[] = {"VK_LAYER_LUNARG_standard_validation"};

        const char* extensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report"};
        checkAvailableExtensions(extensions);

        checkAllAvailableExtensions();

        createVulkanApplication(layers, extensions, "No polygons");

        win32_LoadVulkanExtensions(context);

        setupDebugCallback();

        createWindowSurface(hInstance, windowHandle);

        findCompatiblePhysicalDevice();

        createVulkanDevice(layers);

        vkGetDeviceQueue(context.device, context.presentQueueIdx, 0, &context.presentQueue);

        VkCommandPool commandPool = createCommandPool();
        context.setupCmdBuffer = createCommandBuffer(commandPool);
        context.drawCmdBuffer = createCommandBuffer(commandPool);

        // swap chain creation:

        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        tie(colorFormat, colorSpace) = getColorFormatAndSpace();

        auto&& surfaceCapabilities = getSurfaceCapabilities(context.physicalDevice, context.surface);

        uint32_t desired_image_count = 2;
        checkDesiredImageCount(desired_image_count, surfaceCapabilities);

        VkExtent2D surface_resolution = surfaceCapabilities.currentExtent;
        checkSurfaceResolution(surface_resolution);

        VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
        checkSurfaceTransformFlags(preTransform, surfaceCapabilities);

        VkPresentModeKHR presentationMode = getPresentationMode(context.physicalDevice, context.surface);

        context.swapChain = createSwapchain(context.device, context.surface, desired_image_count, colorFormat,
                                            colorSpace, surface_resolution, preTransform, presentationMode);
        context.presentImages = getSwapchainImages(context.device, context.swapChain);


        // create VkImageViews for our swap chain VkImages buffers:
        VkImageViewCreateInfo presentImagesViewCreateInfo = {};
        presentImagesViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        presentImagesViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        presentImagesViewCreateInfo.format = colorFormat;
        presentImagesViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                                                  VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        presentImagesViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        presentImagesViewCreateInfo.subresourceRange.baseMipLevel = 0;
        presentImagesViewCreateInfo.subresourceRange.levelCount = 1;
        presentImagesViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        presentImagesViewCreateInfo.subresourceRange.layerCount = 1;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence submitFence;
        vkCreateFence(context.device, &fenceCreateInfo, nullptr, &submitFence);

        VkImageView* presentImageViews = new VkImageView[context.presentImages.size()];
        for (auto i = 0; i < context.presentImages.size(); ++i)
        {
            presentImagesViewCreateInfo.image = context.presentImages[i];

            // start recording out image layout change barrier on our setup command buffer:
            vkBeginCommandBuffer(context.setupCmdBuffer, &beginInfo);

            VkImageMemoryBarrier layoutTransitionBarrier = {};
            layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            layoutTransitionBarrier.srcAccessMask = 0;
            layoutTransitionBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutTransitionBarrier.image = context.presentImages[i];
            VkImageSubresourceRange resourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            layoutTransitionBarrier.subresourceRange = resourceRange;

            vkCmdPipelineBarrier(context.setupCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &layoutTransitionBarrier);

            vkEndCommandBuffer(context.setupCmdBuffer);

            submitCommandBufferToQueue(context.presentQueue, context.setupCmdBuffer, submitFence);

            vkWaitForFences(context.device, 1, &submitFence, VK_TRUE, UINT64_MAX);
            vkResetFences(context.device, 1, &submitFence);

            vkResetCommandBuffer(context.setupCmdBuffer, 0);

            auto result = vkCreateImageView(context.device, &presentImagesViewCreateInfo, nullptr,
                                            &presentImageViews[i]);
            checkVulkanResult(result, "Could not create ImageView.");
        }


        // create a depth image:
        VkFormat format = VK_FORMAT_D16_UNORM;
        VkDeviceMemory image_memory;
        uint32_t memory_type_bits;

        context.depthImage = createDepthImage(context.device, context.width, context.height, format);
        tie(image_memory, memory_type_bits) = allocateDeviceMemoryForImage(context.device, context.depthImage);
        bindImageMemory(context.device, context.depthImage, image_memory, 0);

        // before using this depth buffer we must change it's layout:
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(context.setupCmdBuffer, &beginInfo);

            VkImageMemoryBarrier layoutTransitionBarrier = {};
            layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            layoutTransitionBarrier.srcAccessMask = 0;
            layoutTransitionBarrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutTransitionBarrier.image = context.depthImage;
            VkImageSubresourceRange resourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
            layoutTransitionBarrier.subresourceRange = resourceRange;

            vkCmdPipelineBarrier(context.setupCmdBuffer,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 0,
                                 0, NULL,
                                 0, NULL,
                                 1, &layoutTransitionBarrier);

            vkEndCommandBuffer(context.setupCmdBuffer);

            VkPipelineStageFlags waitStageMash[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = NULL;
            submitInfo.pWaitDstStageMask = waitStageMash;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &context.setupCmdBuffer;
            submitInfo.signalSemaphoreCount = 0;
            submitInfo.pSignalSemaphores = NULL;
            auto result = vkQueueSubmit(context.presentQueue, 1, &submitInfo, submitFence);

            vkWaitForFences(context.device, 1, &submitFence, VK_TRUE, UINT64_MAX);
            vkResetFences(context.device, 1, &submitFence);
            vkResetCommandBuffer(context.setupCmdBuffer, 0);
        }

        // create the depth image view:
        context.depthImageView = createDepthImageView(context.device, context.depthImage, format);

        // TUTORIAL_012 - Framebuffers
        // define our attachment points
        context.renderPass = createRenderPass(context.device, colorFormat);

        // create our frame buffers:
        context.frameBuffers = new VkFramebuffer[context.presentImages.size()];
        context.frameBuffers[0] = createFramebuffer(context.device, context.renderPass, context.width, context.height,
                                                    presentImageViews[0], context.depthImageView);
        context.frameBuffers[1] = createFramebuffer(context.device, context.renderPass, context.width, context.height,
                                                    presentImageViews[1], context.depthImageView);


        // create our vertex buffer:
        context.vertexInputBuffer = createVertexInputBuffer(context.device);

        // allocate memory for buffers:
        VkDeviceMemory vertexBufferMemory = allocateDeviceMemoryForBuffer(context.device, context.vertexInputBuffer, memory_type_bits);

        // set buffer content:
        void* mapped;
        auto result = vkMapMemory(context.device, vertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
        checkVulkanResult(result, "Failed to mapp buffer memory.");

        vertex* triangle = (vertex*) mapped;
        vertex v1 = {-1.0f, -1.0f, 0, 1.0f};
        vertex v2 = {1.0f, -1.0f, 0, 1.0f};
        vertex v3 = {0.0f, 1.0f, 0, 1.0f};
        triangle[0] = v1;
        triangle[1] = v2;
        triangle[2] = v3;

        vkUnmapMemory(context.device, vertexBufferMemory);

        result = vkBindBufferMemory(context.device, context.vertexInputBuffer, vertexBufferMemory, 0);
        checkVulkanResult(result, "Failed to bind buffer memory.");


        // TUTORIAL_014 Shaders
        uint32_t codeSize;
        char* code = new char[10000];
        HANDLE fileHandle = 0;

        // load our vertex shader:
        fileHandle = CreateFile("../data/shaders/vert.spv", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            OutputDebugStringA("Failed to open shader file.");
            exit(1);
        }
        ReadFile((HANDLE) fileHandle, code, 10000, (LPDWORD) &codeSize, 0);
        CloseHandle(fileHandle);

        VkShaderModuleCreateInfo vertexShaderCreationInfo = {};
        vertexShaderCreationInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderCreationInfo.codeSize = codeSize;
        vertexShaderCreationInfo.pCode = (uint32_t*) code;

        VkShaderModule vertexShaderModule;
        result = vkCreateShaderModule(context.device, &vertexShaderCreationInfo, NULL, &vertexShaderModule);
        checkVulkanResult(result, "Failed to create vertex shader module.");

        // load our fragment shader:
        fileHandle = CreateFile("../data/shaders/frag.spv", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            OutputDebugStringA("Failed to open shader file.");
            exit(1);
        }
        ReadFile((HANDLE) fileHandle, code, 10000, (LPDWORD) &codeSize, 0);
        CloseHandle(fileHandle);

        VkShaderModuleCreateInfo fragmentShaderCreationInfo = {};
        fragmentShaderCreationInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderCreationInfo.codeSize = codeSize;
        fragmentShaderCreationInfo.pCode = (uint32_t*) code;

        VkShaderModule fragmentShaderModule;
        result = vkCreateShaderModule(context.device, &fragmentShaderCreationInfo, NULL, &fragmentShaderModule);
        checkVulkanResult(result, "Failed to create vertex shader module.");


        // TUTORIAL_015 Graphics Pipeline:
        // empty pipeline layout:
        VkPipelineLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 0;
        layoutCreateInfo.pSetLayouts = NULL;    // Not setting any bindings!
        layoutCreateInfo.pushConstantRangeCount = 0;
        layoutCreateInfo.pPushConstantRanges = NULL;

        result = vkCreatePipelineLayout(context.device, &layoutCreateInfo, NULL, &context.pipelineLayout);
        checkVulkanResult(result, "Failed to create pipeline layout.");

        // setup shader stages:
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
        shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageCreateInfo[0].module = vertexShaderModule;
        shaderStageCreateInfo[0].pName = "main";        // shader entry point function name
        shaderStageCreateInfo[0].pSpecializationInfo = NULL;

        shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageCreateInfo[1].module = fragmentShaderModule;
        shaderStageCreateInfo[1].pName = "main";        // shader entry point function name
        shaderStageCreateInfo[1].pSpecializationInfo = NULL;

        // vertex input configuration:
        VkVertexInputBindingDescription vertexBindingDescription = {};
        vertexBindingDescription.binding = 0;
        vertexBindingDescription.stride = sizeof(vertex);
        vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription vertexAttributeDescritpion = {};
        vertexAttributeDescritpion.location = 0;
        vertexAttributeDescritpion.binding = 0;
        vertexAttributeDescritpion.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vertexAttributeDescritpion.offset = 0;

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;  // attribute indexing is a function of the vertex index
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexAttributeDescritpion;

        // vertex topology config:
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        // viewport config:
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = context.width;
        viewport.height = context.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

        VkRect2D scissors = {};
        scissors.offset = {0, 0};
        scissors.extent = {context.width, context.height};

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissors;

        // rasterization config:
        VkPipelineRasterizationStateCreateInfo rasterizationState = {};
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationState.cullMode = VK_CULL_MODE_NONE;
        rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.depthBiasEnable = VK_FALSE;
        rasterizationState.depthBiasConstantFactor = 0;
        rasterizationState.depthBiasClamp = 0;
        rasterizationState.depthBiasSlopeFactor = 0;
        rasterizationState.lineWidth = 1;

        // sampling config:
        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 0;
        multisampleState.pSampleMask = NULL;
        multisampleState.alphaToCoverageEnable = VK_FALSE;
        multisampleState.alphaToOneEnable = VK_FALSE;

        // depth/stencil config:
        VkStencilOpState noOPStencilState = {};
        noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
        noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
        noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
        noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
        noOPStencilState.compareMask = 0;
        noOPStencilState.writeMask = 0;
        noOPStencilState.reference = 0;

        VkPipelineDepthStencilStateCreateInfo depthState = {};
        depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthState.depthTestEnable = VK_TRUE;
        depthState.depthWriteEnable = VK_TRUE;
        depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthState.depthBoundsTestEnable = VK_FALSE;
        depthState.stencilTestEnable = VK_FALSE;
        depthState.front = noOPStencilState;
        depthState.back = noOPStencilState;
        depthState.minDepthBounds = 0;
        depthState.maxDepthBounds = 0;

        // color blend config: (Actually off for tutorial)
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
        colorBlendAttachmentState.blendEnable = VK_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.colorWriteMask = 0xf;

        VkPipelineColorBlendStateCreateInfo colorBlendState = {};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &colorBlendAttachmentState;
        colorBlendState.blendConstants[0] = 0.0;
        colorBlendState.blendConstants[1] = 0.0;
        colorBlendState.blendConstants[2] = 0.0;
        colorBlendState.blendConstants[3] = 0.0;

        // configure dynamic state:
        VkDynamicState dynamicState[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates = dynamicState;

        // and finally, pipeline config and creation:
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shaderStageCreateInfo;
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pTessellationState = NULL;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pDepthStencilState = &depthState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.layout = context.pipelineLayout;
        pipelineCreateInfo.renderPass = context.renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = 0;

        result = vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL,
                                           &context.pipeline);
        checkVulkanResult(result, "Failed to create graphics pipeline.");


        MSG msg;
        bool done = false;
        while (!done)
        {
            PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
            if (msg.message == WM_QUIT)
            {
                done = true;
            } else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            RedrawWindow(windowHandle, nullptr, nullptr, RDW_INTERNALPAINT);
        }

        vkDestroyDebugReportCallbackEXT(context.instance, context.callback, nullptr);

        return msg.wParam;
    }
    catch (exception& e)
    {
        cout << "==================" << endl;
        cout << e.what() << endl;
    }
}

VkDeviceMemory allocateDeviceMemoryForBuffer(VkDevice device, VkBuffer buffer, uint32_t memory_type_bits)
{
    VkMemoryRequirements requirements = {};
    vkGetBufferMemoryRequirements(device, buffer, &requirements);

    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = requirements.size;

    uint32_t vertex_memory_type_bits = requirements.memoryTypeBits;
    VkMemoryPropertyFlags desired_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    for (uint32_t i = 0; i < 32; ++i)
    {
        VkMemoryType memory_type = context.memoryProperties.memoryTypes[i];
        if (vertex_memory_type_bits & 1)
        {
            if ((memory_type.propertyFlags & desired_memory_flags) == desired_memory_flags)
            {
                allocate_info.memoryTypeIndex = i;
                break;
            }
        }
        memory_type_bits = memory_type_bits >> 1;
    }

    VkDeviceMemory memory;
    auto result = vkAllocateMemory(context.device, &allocate_info, nullptr, &memory);
    checkVulkanResult(result, "Failed to allocate buffer memory.");

    return memory;
}

VkBuffer createVertexInputBuffer(VkDevice device)
{
    VkBufferCreateInfo vertex_buffer_create_info = {};
    vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_create_info.size = sizeof(vertex) * 3;
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertex_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertex_buffer_create_info.queueFamilyIndexCount = 0;
    vertex_buffer_create_info.pQueueFamilyIndices = nullptr;

    VkBuffer buffer;
    auto result = vkCreateBuffer(device, &vertex_buffer_create_info, nullptr, &buffer);
    checkVulkanResult(result, "Failed to create vertex input buffer.");

    return buffer;
}

VkFramebuffer createFramebuffer(VkDevice device,
                                VkRenderPass pass,
                                uint32_t width,
                                uint32_t height,
                                VkImageView color_image_view,
                                VkImageView depth_image_view)
{
    VkImageView attachments[2];
    attachments[0] = color_image_view;
    attachments[1] = depth_image_view;

    VkFramebufferCreateInfo framebuffer_create_info = {};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass = pass;
    framebuffer_create_info.attachmentCount = 2;  // must be equal to the attachment count on render pass
    framebuffer_create_info.pAttachments = attachments;
    framebuffer_create_info.width = width;
    framebuffer_create_info.height = height;
    framebuffer_create_info.layers = 1;

    VkFramebuffer buffer;

    // create a framebuffer per swap chain imageView:
    auto result = vkCreateFramebuffer(context.device, &framebuffer_create_info, NULL, &buffer);
    checkVulkanResult(result, "Failed to create framebuffer.");

    return buffer;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat color_format)
{
    VkAttachmentDescription pass_attachments[2] = {};
    pass_attachments[0].format = color_format;
    pass_attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    pass_attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    pass_attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    pass_attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pass_attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pass_attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    pass_attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    pass_attachments[1].format = VK_FORMAT_D16_UNORM;
    pass_attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    pass_attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    pass_attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pass_attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pass_attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pass_attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    pass_attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_reference = {};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference = {};
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // create the one main subpass of our renderpass:
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    // create our main renderpass:
    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 2;
    render_pass_create_info.pAttachments = pass_attachments;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;

    VkRenderPass render_pass;
    auto result = vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass);
    checkVulkanResult(result, "Failed to create renderpass");

    return render_pass;
}

VkImageView createDepthImageView(VkDevice device, VkImage image, VkFormat format)
{
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange.aspectMask = aspect_mask;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VkImageView view;

    auto result = vkCreateImageView(device, &image_view_create_info, nullptr, &view);
    checkVulkanResult(result, "Failed to create image view.");

    return view;
}

void bindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, int offset)
{
    VkResult result = vkBindImageMemory(device, image, memory, offset);
    checkVulkanResult(result, "Failed to bind image memory.");
}

tuple<VkDeviceMemory, uint32_t> allocateDeviceMemoryForImage(VkDevice device, VkImage image)
{
    VkMemoryRequirements requirements = {};
    vkGetImageMemoryRequirements(device, image, &requirements);
    cout << "Image memory requirements:" << endl;
    cout << "   Size: " << requirements.size << endl;
    cout << "   Alignment: " << requirements.alignment << endl;
    cout << "   Memory type bits: " << requirements.memoryTypeBits << endl;

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = requirements.size;

    // memory_type_bits is a bitfield where if bit i is set, it means that
    // the VkMemoryType i of the VkPhysicalDeviceMemoryProperties structure
    // satisfies the memory requirements:
    uint32_t memory_type_bits = requirements.memoryTypeBits;
    VkMemoryPropertyFlags desired_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t i = 0; i < 32; ++i)
    {
        VkMemoryType memory_type = context.memoryProperties.memoryTypes[i];
        if (memory_type_bits & 1)
        {
            if ((memory_type.propertyFlags & desired_memory_flags) == desired_memory_flags)
            {
                memory_allocate_info.memoryTypeIndex = i;
                break;
            }
        }
        memory_type_bits = memory_type_bits >> 1;
    }


    VkDeviceMemory memory;

    VkResult result = vkAllocateMemory(device, &memory_allocate_info, NULL, &memory);
    checkVulkanResult(result, "Failed to allocate device memory.");

    return tuple<VkDeviceMemory, uint32_t>(memory, memory_type_bits);
}

VkImage createDepthImage(VkDevice device, uint32_t width, uint32_t height, VkFormat format)
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = {width, height, 1};
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    VkResult result = vkCreateImage(device, &imageCreateInfo, nullptr, &image);
    checkVulkanResult(result, "Failed to create depth image.");

    return image;
}

void submitCommandBufferToQueue(VkQueue queue,
                                VkCommandBuffer buffer,
                                VkFence submit_fence)
{
    VkPipelineStageFlags waitStageMash[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitStageMash;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    VkResult result = vkQueueSubmit(context.presentQueue, 1, &submitInfo, submit_fence);
    checkVulkanResult(result, "Could not submit queue.");
}

vector<VkImage> getSwapchainImages(VkDevice device,
                                   VkSwapchainKHR swapchain)
{
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(context.device, context.swapChain, &image_count, nullptr);

    vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(context.device, context.swapChain, &image_count, images.data());

    return images;
}

VkSwapchainKHR createSwapchain(VkDevice device,
                               VkSurfaceKHR surface,
                               uint32_t desired_image_count,
                               VkFormat color_format,
                               VkColorSpaceKHR color_space,
                               VkExtent2D surface_resolution,
                               VkSurfaceTransformFlagBitsKHR pre_transform,
                               VkPresentModeKHR presentation_mode)
{
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = desired_image_count;
    swapChainCreateInfo.imageFormat = color_format;
    swapChainCreateInfo.imageColorSpace = color_space;
    swapChainCreateInfo.imageExtent = surface_resolution;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.preTransform = pre_transform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentation_mode;
    swapChainCreateInfo.clipped = VK_TRUE;     // If we want clipping outside the extents
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;

    auto&& result = vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapchain);
    checkVulkanResult(result, "Failed to create swapchain.");

    return swapchain;
}

VkPresentModeKHR getPresentationMode(VkPhysicalDevice physical_device,
                                     VkSurfaceKHR surface)
{
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    cout << "Present modes: " << present_mode_count << endl;

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.physicalDevice, context.surface, &present_mode_count,
                                              present_modes.data());
    for (auto i = 0; i < present_mode_count; ++i)
        cout << i + 1 << ". " << present_modes[i] << endl;
    cout << "-------------" << endl;

    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;   // always supported.
    auto mailbox_mode_found = find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR);

    if (mailbox_mode_found != present_modes.end())
        mode = VK_PRESENT_MODE_MAILBOX_KHR;

    return mode;
}

void checkSurfaceTransformFlags(VkSurfaceTransformFlagBitsKHR& transform,
                                VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
    printSurfaceCapabilities(capabilities);

    return capabilities;
}

void checkSurfaceResolution(VkExtent2D& surface_resolution)
{
    if (surface_resolution.width == -1)
    {
        surface_resolution.width = context.width;
        surface_resolution.height = context.height;
    } else
    {
        context.width = surface_resolution.width;
        context.height = surface_resolution.height;
    }
    cout << "Corrected width: " << context.width << endl;
    cout << "Corrected height: " << context.height << endl;
}

void printSurfaceCapabilities(const VkSurfaceCapabilitiesKHR& surface_capabilities)
{
    cout << "---------------------" << endl;
    cout << "Surface capabilities:" << endl;
    cout << "Min image count: " << surface_capabilities.minImageCount << endl;
    cout << "Max image count: " << surface_capabilities.maxImageCount << endl;
    cout << "Current extent: " << surface_capabilities.currentExtent.width << "x"
         << surface_capabilities.currentExtent.height << endl;
    cout << "Min image extent: " << surface_capabilities.minImageExtent.width << "x"
         << surface_capabilities.minImageExtent.height << endl;
    cout << "Max image extent: " << surface_capabilities.maxImageExtent.width << "x"
         << surface_capabilities.maxImageExtent.height << endl;
    cout << "Max image array layers: " << surface_capabilities.maxImageArrayLayers << endl;
    cout << "Supported transforms: " << surface_capabilities.supportedTransforms << endl;
    cout << "Current transform: " << surface_capabilities.currentTransform << endl;
    cout << "Supported composite alpha: " << surface_capabilities.supportedCompositeAlpha << endl;
    cout << "Supported usage flags: " << surface_capabilities.supportedUsageFlags << endl;
    cout << "---------------------" << endl;
}

void checkDesiredImageCount(uint32_t& image_count, const VkSurfaceCapabilitiesKHR& surface_capabilities)
{
    if (image_count < surface_capabilities.minImageCount)
    {
        image_count = surface_capabilities.minImageCount;
        return;
    }

    if (surface_capabilities.maxImageCount != 0 && image_count > surface_capabilities.maxImageCount)
    {
        image_count = surface_capabilities.maxImageCount;
    }
}

tuple<VkFormat, VkColorSpaceKHR> getColorFormatAndSpace()
{
    uint32_t surface_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, context.surface, &surface_format_count, nullptr);
    cout << "Surface formats: " << surface_format_count << endl;

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, context.surface, &surface_format_count,
                                         surface_formats.data());
    for (auto& format : surface_formats)
    {
        cout << "Format: " << format.format << endl;
        cout << "Color space: " << format.colorSpace << endl;
        cout << "-----" << endl;
    }

    // If the format list includes just one entry of VK_FORMAT_UNDEFINED, the surface has no preferred format.
    // Otherwise, at least one supported format will be returned.

    VkFormat color_format;
    if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        color_format = VK_FORMAT_B8G8R8_UNORM;
    } else
    {
        color_format = surface_formats[0].format;
    }
    auto color_space = surface_formats[0].colorSpace;

    return tuple<VkFormat, VkColorSpaceKHR>(color_format, color_space);
}

VkCommandBuffer createCommandBuffer(VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
    commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocationInfo.commandPool = commandPool;
    commandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocationInfo.commandBufferCount = 1;

    VkCommandBuffer buffer;

    auto result = vkAllocateCommandBuffers(context.device, &commandBufferAllocationInfo, &buffer);
    checkVulkanResult(result, "Failed to allocate command buffer.");

    return buffer;
}

VkCommandPool createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = context.presentQueueIdx;

    VkCommandPool commandPool;
    checkVulkanResult(vkCreateCommandPool(context.device, &commandPoolCreateInfo, nullptr, &commandPool),
                      "Failed to create command pool.");
    return commandPool;
}

void createVulkanDevice(const char* const* layers)
{
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = context.presentQueueIdx;
    queueCreateInfo.queueCount = 1;
    float queuePriorities[] = {1.0f};
    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queueCreateInfo;
    device_create_info.enabledLayerCount = 1;
    device_create_info.ppEnabledLayerNames = layers;

    const char* deviceExtensions[] = {"VK_KHR_swapchain"};
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = deviceExtensions;

    VkPhysicalDeviceFeatures features = {};
    features.shaderClipDistance = VK_TRUE;
    device_create_info.pEnabledFeatures = &features;

    checkVulkanResult(vkCreateDevice(context.physicalDevice, &device_create_info, nullptr, &context.device),
                      "Failed to create logical device!");
}

void findCompatiblePhysicalDevice()
{
// Find a physical device that has a queue where we can present and do graphics:
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, nullptr);
    cout << "Physical devices: " << physicalDeviceCount << endl;
    VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[physicalDeviceCount];
    vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, physicalDevices);

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        cout << "Device #" << i << ":" << endl;
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
        cout << "   API version: " << deviceProperties.apiVersion << endl;
        cout << "   Driver version: " << deviceProperties.driverVersion << endl;
        cout << "   Vendor ID: " << deviceProperties.vendorID << endl;
        cout << "   Device ID: " << deviceProperties.deviceID << endl;
        cout << "   Device type: " << deviceProperties.deviceType << endl;
        cout << "   Device name: " << deviceProperties.deviceName << endl;
        cout << "   Pipeline cache UUID: " << deviceProperties.pipelineCacheUUID << endl;
        cout << "   Limits: " << endl;
        cout << "       Max image dimension 2D: " << deviceProperties.limits.maxImageDimension2D << endl;
        cout << "       ..." << endl;
        cout << "   Sparse properties: " << endl;
        cout << "       Residency aligned mip size: " << deviceProperties.sparseProperties.residencyAlignedMipSize
             << endl;
        cout << "       ..." << endl;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, NULL);
        VkQueueFamilyProperties* queueFamilyProperties = new VkQueueFamilyProperties[queueFamilyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties);
        cout << i << ". Queue families: " << queueFamilyCount << endl;

        for (uint32_t j = 0; j < queueFamilyCount; ++j)
        {
            cout << "   Queue count: " << queueFamilyProperties[j].queueCount << endl;
            cout << "   Queue flags: " << queueFamilyProperties[j].queueFlags << endl;
            cout << "   Timestamp valid bits: " << queueFamilyProperties[j].timestampValidBits << endl;
            cout << "   Min image transfer granularity: "
                 << queueFamilyProperties[j].minImageTransferGranularity.width << " "
                 << queueFamilyProperties[j].minImageTransferGranularity.height << " "
                 << queueFamilyProperties[j].minImageTransferGranularity.depth << endl;

            VkBool32 supportsPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, context.surface, &supportsPresent);

            if (supportsPresent &&
                queueFamilyProperties[j].queueFlags & (VK_QUEUE_GRAPHICS_BIT || VK_QUEUE_COMPUTE_BIT))
            {
                context.physicalDevice = physicalDevices[i];
                context.physicalDeviceProperties = deviceProperties;
                context.presentQueueIdx = j;
            }
        }

        delete[] queueFamilyProperties;

        if (context.physicalDevice)
        {
            break;
        }
    }
    delete[] physicalDevices;

    assert(context.physicalDevice, "No physical device present that can render and present!");

    // Fill up the physical device memory properties:
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &context.memoryProperties);
}

void createWindowSurface(HINSTANCE hInstance, HWND windowHandle)
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = hInstance;
    surfaceCreateInfo.hwnd = windowHandle;

    checkVulkanResult(vkCreateWin32SurfaceKHR(context.instance, &surfaceCreateInfo, NULL, &context.surface),
                      "Could not create surface.");
}

void setupDebugCallback()
{
    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                               VK_DEBUG_REPORT_WARNING_BIT_EXT |
                               VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
    callbackCreateInfo.pUserData = NULL;

    checkVulkanResult(vkCreateDebugReportCallbackEXT(context.instance, &callbackCreateInfo, NULL, &context.callback),
                      "Failed to create debug report callback.");
}

void createVulkanApplication(const char* const* layers,
                             const char* const* extensions,
                             const char* application_name)
{
    VkResult result;
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = application_name;
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 24);

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = 1;
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledExtensionCount = 3;
    instanceInfo.ppEnabledExtensionNames = extensions;

    checkVulkanResult(vkCreateInstance(&instanceInfo, NULL, &context.instance), "Failed to create vulkan instance.");
}

void checkAvailableExtensions(const char* const* extensions)
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensionsAvailable = new VkExtensionProperties[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionsAvailable);
    //cout << "Extensions available: " << extensionCount << endl;
    uint32_t foundExtensions = 0;
    for (int i = 0; i < extensionCount; ++i)
    {
        if (strcmp(extensionsAvailable[i].extensionName, extensions[i]) == 0)
        {
            foundExtensions++;
        }
    }
    assert(foundExtensions == 3, "Could not find debug extension");
}

void checkAvailableValidationLayers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties* layersAvailable = new VkLayerProperties[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable);

    bool foundValidation = false;
    for (int i = 0; i < layerCount; ++i)
    {
        if (strcmp(layersAvailable[i].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
        {
            foundValidation = true;
        }
    }
    assert(foundValidation, "Could not find validation layer.");
}

void checkAllAvailableExtensions()
{
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    cout << "Available layers: " << layer_count << endl;

    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    for (const auto& layer : layers)
    {
        cout << "Name: " << layer.layerName << endl;
        cout << "Description: " << layer.description << endl;
        cout << "Implementation version: " << layer.implementationVersion << endl;
        cout << "Specification version: " << layer.specVersion << endl;

        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(layer.layerName, &extension_count, nullptr);
        cout << "Available extensions: " << extension_count << endl;

        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(layer.layerName, &extension_count, extensions.data());
        for (const auto& extension : extensions)
        {
            cout << "   Name: " << extension.extensionName << endl;
            cout << "   Specification version: " << extension.specVersion << endl;
            cout << "   -----" << endl;
        }
        cout << "-----------------------------" << endl;
    }

    uint32_t implicit_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &implicit_extension_count, nullptr);
    cout << "Extensions from implicitly implemented layers: " << endl;

    std::vector<VkExtensionProperties> implicit_extensions(implicit_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &implicit_extension_count, implicit_extensions.data());
    for (const auto& extension : implicit_extensions)
    {
        cout << "   Name: " << extension.extensionName << endl;
        cout << "   Specification version: " << extension.specVersion << endl;
        cout << "   -----" << endl;
    }
}

HWND createWindow(HINSTANCE hInstance, const char* window_caption)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = "VulkanWindowClass";
    RegisterClassEx(&windowClass);

    context.width = 800;
    context.height = 600;

    HWND windowHandle = CreateWindowEx(0,
                                       "VulkanWindowClass",
                                       window_caption,
                                       WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                       100,
                                       100,
                                       context.width,
                                       context.height,
                                       nullptr,
                                       nullptr,
                                       hInstance,
                                       nullptr);
    return windowHandle;
}