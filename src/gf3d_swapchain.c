#include "gf3d_swapchain.h"
#include "gf3d_vqueues.h"

#include "simple_logger.h"

typedef struct
{
    VkSwapchainKHR              swapChain;
    VkDevice                    device;
    VkSurfaceCapabilitiesKHR    capabilities;
    Uint32                      formatCount;
    VkSurfaceFormatKHR         *formats;
    Uint32                      presentModeCount;
    VkPresentModeKHR           *presentModes;
    int                         chosenFormat;
    int                         chosenPresentMode;
    VkExtent2D                  extent;                 // resolution of the swap buffers
    Uint32                      swapChainCount;
}vSwapChain;

static vSwapChain gf3d_swapchain = {0};

void gf2d_swapchain_create(VkDevice device,VkSurfaceKHR surface);
void gf3d_swapchain_close();
int gf3d_swapchain_get_format();
int gf3d_swapchain_get_presentation_mode();
VkExtent2D gf3d_swapchain_get_extent(Uint32 width,Uint32 height);

void gf3d_swapchain_init(VkPhysicalDevice device,VkDevice logicalDevice,VkSurfaceKHR surface,Uint32 width,Uint32 height)
{
    int i;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &gf3d_swapchain.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, NULL);

    slog("device supports %i surface formats",gf3d_swapchain.formatCount);
    if (gf3d_swapchain.formatCount != 0)
    {
        gf3d_swapchain.formats = (VkSurfaceFormatKHR*)gf3d_allocate_array(sizeof(VkSurfaceFormatKHR),gf3d_swapchain.formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, gf3d_swapchain.formats);
        for (i = 0; i < gf3d_swapchain.formatCount; i++)
        {
            slog("surface format %i:",i);
            slog("format: %i",gf3d_swapchain.formats[i].format);
            slog("colorspace: %i",gf3d_swapchain.formats[i].colorSpace);
        }
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &gf3d_swapchain.presentModeCount, NULL);

    slog("device supports %i presentation modes",gf3d_swapchain.presentModeCount);
    if (gf3d_swapchain.presentModeCount != 0)
    {
        gf3d_swapchain.presentModes = (VkPresentModeKHR*)gf3d_allocate_array(sizeof(VkPresentModeKHR),gf3d_swapchain.presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &gf3d_swapchain.presentModeCount, gf3d_swapchain.presentModes);
        for (i = 0; i < gf3d_swapchain.presentModeCount; i++)
        {
            slog("presentation mode: %i is %i",i,gf3d_swapchain.presentModes[i]);
        }
    }
    
    gf3d_swapchain.chosenFormat = gf3d_swapchain_get_format();
    slog("chosing surface format %i",gf3d_swapchain.chosenFormat);
    
    gf3d_swapchain.chosenPresentMode = gf3d_swapchain_get_presentation_mode();
    slog("chosing presentation mode %i",gf3d_swapchain.chosenPresentMode);
    
    gf3d_swapchain.extent = gf3d_swapchain_get_extent(width,height);
    slog("chosing swap chain extent of (%i,%i)",gf3d_swapchain.extent.width,gf3d_swapchain.extent.height);
    
    gf2d_swapchain_create(logicalDevice,surface);
    gf3d_swapchain.device = logicalDevice;

}

void gf2d_swapchain_create(VkDevice device,VkSurfaceKHR surface)
{
    Sint32 graphicsFamily;
    Sint32 presentFamily;
    VkSwapchainCreateInfoKHR createInfo = {0};
    Uint32 queueFamilyIndices[2];
    
    slog("minimum images needed for swap chain: %i",gf3d_swapchain.capabilities.minImageCount);
    slog("Maximum images needed for swap chain: %i",gf3d_swapchain.capabilities.maxImageCount);
    gf3d_swapchain.swapChainCount = gf3d_swapchain.capabilities.minImageCount + 1;
    if (gf3d_swapchain.capabilities.maxImageCount)gf3d_swapchain.swapChainCount = MIN(gf3d_swapchain.swapChainCount,gf3d_swapchain.capabilities.maxImageCount);
    slog("using %i images for the swap chain",gf3d_swapchain.swapChainCount);
    
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = gf3d_swapchain.swapChainCount;
    createInfo.imageFormat = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format;
    createInfo.imageColorSpace = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].colorSpace;
    createInfo.imageExtent = gf3d_swapchain.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    graphicsFamily = gf3d_vqueues_get_graphics_queue_family();
    presentFamily = gf3d_vqueues_get_present_queue_family();
    queueFamilyIndices[0] = graphicsFamily;
    queueFamilyIndices[1] = presentFamily;
    
    if (graphicsFamily != presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }
    createInfo.preTransform = gf3d_swapchain.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // our window is opaque, but it doesn't have to be

    createInfo.presentMode = gf3d_swapchain.presentModes[gf3d_swapchain.chosenPresentMode];
    createInfo.clipped = VK_TRUE;
    
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &gf3d_swapchain.swapChain) != VK_SUCCESS)
    {
        slog("failed to create swap chain!");
    }
}

VkExtent2D gf3d_swapchain_get_extent(Uint32 width,Uint32 height)
{
    VkExtent2D actualExtent;
    slog("Requested resolution: (%i,%i)",width,height);
    slog("Minimum resolution: (%i,%i)",gf3d_swapchain.capabilities.minImageExtent.width,gf3d_swapchain.capabilities.minImageExtent.height);
    slog("Maximum resolution: (%i,%i)",gf3d_swapchain.capabilities.maxImageExtent.width,gf3d_swapchain.capabilities.maxImageExtent.height);
    
    actualExtent.width = MAX(gf3d_swapchain.capabilities.minImageExtent.width,MIN(width,gf3d_swapchain.capabilities.maxImageExtent.width));
    actualExtent.height = MAX(gf3d_swapchain.capabilities.minImageExtent.height,MIN(height,gf3d_swapchain.capabilities.maxImageExtent.height));
    return actualExtent;
}

int gf3d_swapchain_get_presentation_mode()
{
    int i;
    int chosen = -1;
    for (i = 0; i < gf3d_swapchain.formatCount; i++)
    {
        if (gf3d_swapchain.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return i;
        chosen = i;
    }
    return chosen;
}

int gf3d_swapchain_get_format()
{
    int i;
    int chosen = -1;
    for (i = 0; i < gf3d_swapchain.formatCount; i++)
    {
        if ((gf3d_swapchain.formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
            (gf3d_swapchain.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            return i;
        chosen = i;
    }
    return chosen;
}

void gf3d_swapchain_close()
{
    vkDestroySwapchainKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, NULL);
    if (gf3d_swapchain.formats)
    {
        free(gf3d_swapchain.formats);
    }
    if (gf3d_swapchain.presentModes)
    {
        free(gf3d_swapchain.presentModes);
    }
    memset(&gf3d_swapchain,0,sizeof(vSwapChain));
}

Bool gf3d_swapchain_validation_check()
{
    if (!gf3d_swapchain.presentModeCount)
    {
        slog("swapchain has no usable presentation modes");
        return false;
    }
    if (!gf3d_swapchain.formatCount)
    {
        slog("swapchain has no usable surface formats");
        return false;
    }
    return true;
}
/*eol@eof*/