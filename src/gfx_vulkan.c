#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

typedef struct Gfx_Vulkan_Queue Gfx_Vulkan_Queue;
typedef struct Gfx_Vulkan_Meta  Gfx_Vulkan_Meta;

struct Gfx_Vulkan_Queue {
    int32_t graphics;
    int32_t present;
};

struct Gfx_Vulkan_Meta {
    VkInstance             instance;
    VkPhysicalDevice       physical_device;
    VkDevice               logical_device;
    VkQueue                gfx_queue;
    VkQueue                present_queue;
    VkSurfaceKHR           surface;
    VkSurfaceFormatKHR     chosen_format;
    VkPresentModeKHR       chosen_present_mode;
    VkExtent2D             chosen_swap_extent;
    VkSwapchainKHR         swap_chain;
    VkImage *              swap_chain_images;
    uint32_t               num_swap_chain_images;
};

typedef struct {
    VkSurfaceCapabilitiesKHR    capabilities;
    uint32_t                    num_formats;
    VkSurfaceFormatKHR *        formats;
    uint32_t                    num_present_modes;
    VkPresentModeKHR   *        present_modes;
} Gfx_Vulkan_Swap_Chain_Details;

bool
gfx_init(Gfx *gfx, Os *os, Mem_Arena *arena) {
    Gfx_Vulkan_Meta *meta = ALLOC_STRUCT(arena, Gfx_Vulkan_Meta);
    Os_Win32Meta *os_meta = (Os_Win32Meta *)os->meta;

    /* erweiterungen {{{ */
    char *required_extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
    };
    uint32_t num_required_extensions = ARRAY_SIZE(required_extensions);

    uint32_t num_extensions = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, NULL);
    VkExtensionProperties *extensions = ALLOC_SIZE(arena, sizeof(VkExtensionProperties)*num_extensions);
    vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, extensions);

    bool extensions_supported = true;
    for ( uint32_t required_extensions_idx = 0;
          required_extensions_idx < num_required_extensions;
          ++required_extensions_idx )
    {
        bool extension_found = false;

        char *required_extension = required_extensions[required_extensions_idx];
        for ( uint32_t extensions_idx = 0; extensions_idx < num_extensions; ++extensions_idx ) {
            VkExtensionProperties extension = extensions[extensions_idx];

            if ( is_equal(required_extension, extension.extensionName, string_size(required_extension)) ) {
                extension_found = true;
                break;
            }
        }

        if ( !extension_found ) {
            return false;
        }
    }
    /* }}} */
    /* validierung {{{ */
    char *validation_layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    uint32_t num_layers;
    vkEnumerateInstanceLayerProperties(&num_layers, NULL);
    VkLayerProperties *available_layers = ALLOC_SIZE(arena, sizeof(VkLayerProperties)*num_layers);
    vkEnumerateInstanceLayerProperties(&num_layers, available_layers);

    bool validation_supported = true;
    for ( int validation_idx = 0; validation_idx < ARRAY_SIZE(validation_layers); ++validation_idx ) {
        char *validation_layer = validation_layers[validation_idx];
        bool layer_found = false;

        for ( uint32_t layer_idx = 0; layer_idx < num_layers; ++layer_idx ) {
            VkLayerProperties layer = available_layers[layer_idx];

            if ( is_equal(validation_layer, layer.layerName, string_size(validation_layer)) ) {
                layer_found = true;
                break;
            }
        }

        if ( !layer_found ) {
            validation_supported = false;
        }
    }
    /* }}} */
    /* app info {{{ */
    VkApplicationInfo app_info = {0};

    app_info.sType                   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName        = "HelloTriangle";
    app_info.applicationVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName             = "NoEngine";
    app_info.engineVersion           = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion              = VK_API_VERSION_1_2;
    /* }}} */
    /* instanz {{{ */
    VkInstanceCreateInfo inst_info = {0};

    inst_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pApplicationInfo        = &app_info;
    inst_info.enabledExtensionCount   = num_required_extensions;
    inst_info.ppEnabledExtensionNames = required_extensions;

    if ( validation_supported ) {
        inst_info.enabledLayerCount = ARRAY_SIZE(validation_layers);
        inst_info.ppEnabledLayerNames = validation_layers;
    }

    VkResult result = vkCreateInstance(&inst_info, NULL, &meta->instance);

    if ( result != VK_SUCCESS ) {
        return false;
    }
    /* }}} */
    /* oberfläche {{{ */
    VkWin32SurfaceCreateInfoKHR surface_create_info = {0};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd  = os_meta->window_handle;
    surface_create_info.hinstance = GetModuleHandle(NULL);

    if ( vkCreateWin32SurfaceKHR(meta->instance, &surface_create_info, NULL, &meta->surface) != VK_SUCCESS ) {
        return false;
    }
    /* }}} */
    /* physisches gerät {{{ */
    meta->physical_device = VK_NULL_HANDLE;

    uint32_t num_devices = 0;
    vkEnumeratePhysicalDevices(meta->instance, &num_devices, NULL);
    if ( !num_devices ) {
        return false;
    }

    VkPhysicalDevice *devices = ALLOC_SIZE(arena, sizeof(VkPhysicalDevice)*num_devices);
    vkEnumeratePhysicalDevices(meta->instance, &num_devices, devices);

    char *required_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    uint32_t num_required_device_extensions = ARRAY_SIZE(required_device_extensions);

    for ( uint32_t device_idx = 0; device_idx < num_devices; ++device_idx ) {
        VkPhysicalDevice d = devices[device_idx];

        VkPhysicalDeviceProperties device_prop;
        VkPhysicalDeviceFeatures   device_feat;
        vkGetPhysicalDeviceProperties(d, &device_prop);
        vkGetPhysicalDeviceFeatures(d, &device_feat);

        uint32_t num_device_extensions;
        vkEnumerateDeviceExtensionProperties(d, NULL, &num_device_extensions, NULL);
        VkExtensionProperties *available_device_extensions = ALLOC_SIZE(arena, sizeof(VkExtensionProperties)*num_device_extensions);
        vkEnumerateDeviceExtensionProperties(d, NULL, &num_device_extensions, available_device_extensions);
        for ( uint32_t required_device_extension_idx = 0;
                required_device_extension_idx < num_required_device_extensions;
                ++required_device_extension_idx )
        {
            char *required_device_extension = required_device_extensions[required_device_extension_idx];
            bool required_device_extension_found = false;

            for ( uint32_t device_extension_idx = 0; device_extension_idx < num_device_extensions; ++device_extension_idx ) {
                VkExtensionProperties device_extension = available_device_extensions[device_extension_idx];

                if ( is_equal( required_device_extension, device_extension.extensionName, string_size(required_device_extension) ) ) {
                    required_device_extension_found = true;
                    break;
                }
            }

            if ( !required_device_extension_found ) {
                return false;
            }
        }

        if ( device_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
             device_feat.geometryShader )
        {
            meta->physical_device = d;
        }
    }

    if (meta->physical_device == VK_NULL_HANDLE) {
        return false;
    }
    /* }}} */
    /* auftragsschlangen {{{ */
    uint32_t num_queues_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(meta->physical_device, &num_queues_families, NULL);
    VkQueueFamilyProperties *queue_families = ALLOC_SIZE(arena, sizeof(VkQueueFamilyProperties)*num_queues_families);
    vkGetPhysicalDeviceQueueFamilyProperties(meta->physical_device, &num_queues_families, queue_families);

    Gfx_Vulkan_Queue queue_index = { -1, -1 };
    VkBool32 present_support = false;
    for ( uint32_t queue_idx = 0; queue_idx < num_queues_families; ++queue_idx ) {
        VkQueueFamilyProperties queue_family = queue_families[queue_idx];
        if ( queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
            queue_index.graphics = (int32_t)queue_idx;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(meta->physical_device, queue_idx, meta->surface, &present_support);
        if ( present_support && queue_index.present == -1 && queue_index.present != queue_index.graphics ) {
            queue_index.present = (int32_t)queue_idx;
        }
    }

    if ( queue_index.graphics == -1 || queue_index.present == -1 ) {
        return false;
    }
    /* }}} */
    /* logisches gerät {{{ */
    VkDeviceQueueCreateInfo queue_info[2] = {0};
    float queue_prio = 1.0f;

    /* gfx queue {{{ */
    queue_info[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = queue_index.graphics;
    queue_info[0].queueCount       = 1;
    queue_info[0].pQueuePriorities = &queue_prio;
    /* }}} */
    /* present queue {{{ */
    queue_info[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[1].queueFamilyIndex = queue_index.present;
    queue_info[1].queueCount       = 1;
    queue_info[1].pQueuePriorities = &queue_prio;
    /* }}} */

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info      = {0};
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos       = queue_info;
    create_info.queueCreateInfoCount    = 2;
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = (uint32_t)num_required_device_extensions;
    create_info.ppEnabledExtensionNames = required_device_extensions;

    if ( vkCreateDevice(meta->physical_device, &create_info, NULL, &meta->logical_device) != VK_SUCCESS ) {
        return false;
    }

    vkGetDeviceQueue(meta->logical_device, queue_index.graphics, 0, &meta->gfx_queue);
    vkGetDeviceQueue(meta->logical_device, queue_index.present,  0, &meta->present_queue);
    /* }}} */
    /* swap chain {{{ */
    Gfx_Vulkan_Swap_Chain_Details swap_chain_details = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(meta->physical_device, meta->surface,
            &swap_chain_details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(meta->physical_device, meta->surface,
            &swap_chain_details.num_formats, NULL);

    if ( swap_chain_details.num_formats ) {
        swap_chain_details.formats = ALLOC_SIZE(arena, sizeof(VkSurfaceFormatKHR)*swap_chain_details.num_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(meta->physical_device, meta->surface,
                &swap_chain_details.num_formats, swap_chain_details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(meta->physical_device, meta->surface,
            &swap_chain_details.num_present_modes, NULL);

    if ( swap_chain_details.num_present_modes ) {
        swap_chain_details.present_modes = ALLOC_SIZE(arena, sizeof(VkPresentModeKHR)*swap_chain_details.num_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(meta->physical_device, meta->surface,
                &swap_chain_details.num_present_modes, swap_chain_details.present_modes);
    }

    if ( !swap_chain_details.num_formats || !swap_chain_details.num_present_modes ) {
        return false;
    }

    meta->chosen_format = swap_chain_details.formats[0];
    for ( uint32_t formats_idx = 0; formats_idx < swap_chain_details.num_formats; ++formats_idx ) {
        VkSurfaceFormatKHR format = swap_chain_details.formats[formats_idx];

        if ( format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
            meta->chosen_format = format;
        }
    }

    meta->chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for ( uint32_t present_mode_idx = 0; present_mode_idx < swap_chain_details.num_present_modes; ++present_mode_idx ) {
        VkPresentModeKHR present_mode = swap_chain_details.present_modes[present_mode_idx];
        if ( present_mode == VK_PRESENT_MODE_MAILBOX_KHR ) {
            meta->chosen_present_mode = present_mode;
        }
    }

    if ( swap_chain_details.capabilities.currentExtent.width != UINT32_MAX ) {
        meta->chosen_swap_extent = swap_chain_details.capabilities.currentExtent;
    } else {
        VkExtent2D actual_extent = { 1024, 860 };
        actual_extent.width =
            MAX(
                swap_chain_details.capabilities.minImageExtent.width,
                MIN(
                    swap_chain_details.capabilities.maxImageExtent.width,
                    actual_extent.width
                )
            );

        actual_extent.height =
            MAX(
                swap_chain_details.capabilities.minImageExtent.height,
                MIN(
                    swap_chain_details.capabilities.maxImageExtent.height,
                    actual_extent.height
                )
            );

        meta->chosen_swap_extent = actual_extent;
    }

    uint32_t image_count = swap_chain_details.capabilities.minImageCount + 1;
    if ( swap_chain_details.capabilities.maxImageCount > 0 && image_count >
            swap_chain_details.capabilities.maxImageCount )
    {
        image_count = swap_chain_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swap_chain_create_info = {0};
    swap_chain_create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface          = meta->surface;
    swap_chain_create_info.minImageCount    = image_count;
    swap_chain_create_info.imageFormat      = meta->chosen_format.format;
    swap_chain_create_info.imageColorSpace  = meta->chosen_format.colorSpace;
    swap_chain_create_info.imageExtent      = meta->chosen_swap_extent;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if ( queue_index.graphics != queue_index.present ) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        swap_chain_create_info.pQueueFamilyIndices = (uint32_t[]){ queue_index.graphics, queue_index.present };
    } else {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_create_info.queueFamilyIndexCount = 0;
        swap_chain_create_info.pQueueFamilyIndices = NULL;
    }

    swap_chain_create_info.preTransform   = swap_chain_details.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode    = meta->chosen_present_mode;
    swap_chain_create_info.clipped        = VK_TRUE;
    swap_chain_create_info.oldSwapchain   = VK_NULL_HANDLE;

    if ( vkCreateSwapchainKHR(meta->logical_device, &swap_chain_create_info, NULL, &meta->swap_chain) != VK_SUCCESS ) {
        return false;
    }

    vkGetSwapchainImagesKHR(meta->logical_device, meta->swap_chain, &meta->num_swap_chain_images, NULL);
    meta->swap_chain_images = ALLOC_SIZE(arena, sizeof(VkImage)*meta->num_swap_chain_images);
    vkGetSwapchainImagesKHR(meta->logical_device, meta->swap_chain, &meta->num_swap_chain_images, meta->swap_chain_images);
    /* }}} */

    return true;
}

void
gfx_cleanup(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    vkDestroySwapchainKHR(meta->logical_device, meta->swap_chain, NULL);
    vkDestroySurfaceKHR(meta->instance, meta->surface, NULL);
    vkDestroyInstance(meta->instance, NULL);
    vkDestroyDevice(meta->logical_device, NULL);
}

void
gfx_rect(Rect rect, Color color) {
}

