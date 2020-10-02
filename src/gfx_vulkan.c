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
    VkSurfaceFormatKHR     format;
    VkPresentModeKHR       present_mode;
    VkExtent2D             swap_chain_extent;
    VkSwapchainKHR         swap_chain;
    VkImage             *  swap_chain_images;
    uint32_t               num_swap_chain_images;
    VkImageView         *  image_views;
    uint32_t               num_image_views;
    VkFramebuffer       *  framebuffers;
    uint32_t               num_framebuffers;
    VkRenderPass           render_pass;
    VkPipelineLayout       pipeline_layout;
    VkPipeline             pipeline;
    VkCommandPool          command_pool;
    VkCommandBuffer     *  command_buffers;
    uint32_t               num_command_buffers;
    VkSemaphore            image_available_semaphore;
    VkSemaphore            render_finished_semaphore;
};

typedef struct {
    VkSurfaceCapabilitiesKHR    capabilities;
    uint32_t                    num_formats;
    VkSurfaceFormatKHR *        formats;
    uint32_t                    num_present_modes;
    VkPresentModeKHR   *        present_modes;
} Gfx_Vulkan_Swap_Chain_Details;

/* gfx api {{{ */
GFX_API_INIT() {
    Gfx_Vulkan_Meta *meta = ALLOC_STRUCT(gfx->perm_arena, Gfx_Vulkan_Meta);
    gfx->meta = meta;
    Os_Win32_Meta *os_meta = (Os_Win32_Meta *)os->meta;

    /* erweiterungen {{{ */
    char *required_extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
    };
    uint32_t num_required_extensions = ARRAY_SIZE(required_extensions);

    uint32_t num_extensions = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &num_extensions, NULL);
    VkExtensionProperties *extensions = ALLOC_SIZE(gfx->temp_arena, sizeof(VkExtensionProperties)*num_extensions);
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
            gfx->msg = "benötigte erweiterungen nicht gefunden";

            return GFX_FAILURE;
        }
    }
    /* }}} */
    /* validierung {{{ */
    char *validation_layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    uint32_t num_layers;
    vkEnumerateInstanceLayerProperties(&num_layers, NULL);
    VkLayerProperties *available_layers = ALLOC_SIZE(gfx->temp_arena, sizeof(VkLayerProperties)*num_layers);
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

    if ( vkCreateInstance(&inst_info, NULL, &meta->instance) != VK_SUCCESS ) {
        gfx->msg = "vulkan instanz konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* oberfläche {{{ */
    VkWin32SurfaceCreateInfoKHR surface_create_info = {0};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd  = os_meta->window_handle;
    surface_create_info.hinstance = GetModuleHandle(NULL);

    if ( vkCreateWin32SurfaceKHR(meta->instance, &surface_create_info, NULL, &meta->surface) != VK_SUCCESS ) {
        gfx->msg = "vulkan oberfläche konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* physisches gerät {{{ */
    meta->physical_device = VK_NULL_HANDLE;

    uint32_t num_devices = 0;
    vkEnumeratePhysicalDevices(meta->instance, &num_devices, NULL);
    if ( !num_devices ) {
        gfx->msg = "keine physischen geräte verfügbar";

        return GFX_FAILURE;
    }

    VkPhysicalDevice *devices = ALLOC_SIZE(gfx->temp_arena, sizeof(VkPhysicalDevice)*num_devices);
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
        VkExtensionProperties *available_device_extensions = ALLOC_SIZE(gfx->temp_arena, sizeof(VkExtensionProperties)*num_device_extensions);
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
                gfx->msg = "notwendige geräterweiterungen nicht verfügbar";

                return GFX_FAILURE;
            }
        }

        if ( device_prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
             device_feat.geometryShader )
        {
            meta->physical_device = d;
        }
    }

    if (meta->physical_device == VK_NULL_HANDLE) {
        gfx->msg = "kein passendes physisches gerät konnte gefunden werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* auftragsschlangen {{{ */
    uint32_t num_queues_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(meta->physical_device, &num_queues_families, NULL);
    VkQueueFamilyProperties *queue_families = ALLOC_SIZE(gfx->temp_arena, sizeof(VkQueueFamilyProperties)*num_queues_families);
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
        gfx->msg = "notwendigen warteschlangen konnten nicht ermittelt werden";

        return GFX_FAILURE;
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
        gfx->msg = "logisches gerät konnte nicht erstellt werden";

        return GFX_FAILURE;
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
        swap_chain_details.formats = ALLOC_SIZE(gfx->temp_arena, sizeof(VkSurfaceFormatKHR)*swap_chain_details.num_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(meta->physical_device, meta->surface,
                &swap_chain_details.num_formats, swap_chain_details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(meta->physical_device, meta->surface,
            &swap_chain_details.num_present_modes, NULL);

    if ( swap_chain_details.num_present_modes ) {
        swap_chain_details.present_modes = ALLOC_SIZE(gfx->temp_arena, sizeof(VkPresentModeKHR)*swap_chain_details.num_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(meta->physical_device, meta->surface,
                &swap_chain_details.num_present_modes, swap_chain_details.present_modes);
    }

    if ( !swap_chain_details.num_formats || !swap_chain_details.num_present_modes ) {
        gfx->msg = "notwendige swap chain eigenschaften konnten nicht ermittelt werden";

        return GFX_FAILURE;
    }

    meta->format = swap_chain_details.formats[0];
    for ( uint32_t formats_idx = 0; formats_idx < swap_chain_details.num_formats; ++formats_idx ) {
        VkSurfaceFormatKHR format = swap_chain_details.formats[formats_idx];

        if ( format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
            meta->format = format;
        }
    }

    meta->present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for ( uint32_t present_mode_idx = 0; present_mode_idx < swap_chain_details.num_present_modes; ++present_mode_idx ) {
        VkPresentModeKHR present_mode = swap_chain_details.present_modes[present_mode_idx];
        if ( present_mode == VK_PRESENT_MODE_MAILBOX_KHR ) {
            meta->present_mode = present_mode;
        }
    }

    if ( swap_chain_details.capabilities.currentExtent.width != UINT32_MAX ) {
        meta->swap_chain_extent = swap_chain_details.capabilities.currentExtent;
    } else {
        VkExtent2D actual_extent = { os->window.dim.width, os->window.dim.height };
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

        meta->swap_chain_extent = actual_extent;
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
    swap_chain_create_info.imageFormat      = meta->format.format;
    swap_chain_create_info.imageColorSpace  = meta->format.colorSpace;
    swap_chain_create_info.imageExtent      = meta->swap_chain_extent;
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
    swap_chain_create_info.presentMode    = meta->present_mode;
    swap_chain_create_info.clipped        = VK_TRUE;
    swap_chain_create_info.oldSwapchain   = VK_NULL_HANDLE;

    if ( vkCreateSwapchainKHR(meta->logical_device, &swap_chain_create_info, NULL, &meta->swap_chain) != VK_SUCCESS ) {
        gfx->msg = "swap chain konnte nicht erstellt werden";

        return GFX_FAILURE;
    }

    vkGetSwapchainImagesKHR(meta->logical_device, meta->swap_chain, &meta->num_swap_chain_images, NULL);
    meta->swap_chain_images = ALLOC_SIZE(gfx->perm_arena, sizeof(VkImage)*meta->num_swap_chain_images);
    vkGetSwapchainImagesKHR(meta->logical_device, meta->swap_chain, &meta->num_swap_chain_images, meta->swap_chain_images);
    /* }}} */
    /* image views {{{ */
    meta->num_image_views = meta->num_swap_chain_images;
    meta->image_views = ALLOC_SIZE(gfx->perm_arena, sizeof(VkImageView)*meta->num_image_views);

    for ( uint32_t image_view_idx = 0; image_view_idx < meta->num_swap_chain_images; ++image_view_idx )
    {
        VkImageViewCreateInfo image_view_create_info = {0};

        image_view_create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image    = meta->swap_chain_images[image_view_idx];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format   = meta->format.format;

        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel   = 0;
        image_view_create_info.subresourceRange.levelCount     = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount     = 1;

        if ( vkCreateImageView(meta->logical_device, &image_view_create_info, NULL, meta->image_views + image_view_idx) != VK_SUCCESS ) {
            gfx->msg = "image view konnte nicht erstellt werden";

            return GFX_FAILURE;
        }
    }
    /* }}} */
    /* render pass {{{ */
    VkAttachmentDescription color_attachment = {0};

    color_attachment.format         = meta->format.format;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};

    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};

    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    VkSubpassDependency dependency = {0};

    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {0};

    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments    = &color_attachment;
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &dependency;

    if ( vkCreateRenderPass(meta->logical_device, &render_pass_info, NULL, &meta->render_pass) != VK_SUCCESS ) {
        gfx->msg = "render pass konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* grafikpipeline {{{ */
    Gfx_Shader gfx_vert_shader = gfx_shader(gfx, "shader/vulkan_vert.spv", gfx->perm_arena);
    VkShaderModule vert_shader = *(VkShaderModule *)gfx_vert_shader.meta;
    Gfx_Shader gfx_frag_shader = gfx_shader(gfx, "shader/vulkan_frag.spv", gfx->perm_arena);
    VkShaderModule frag_shader = *(VkShaderModule *)gfx_frag_shader.meta;

    VkPipelineShaderStageCreateInfo shader_stage_create_info[2] = {0};

    shader_stage_create_info[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stage_create_info[0].module = vert_shader;
    shader_stage_create_info[0].pName  = "main";

    shader_stage_create_info[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stage_create_info[1].module = frag_shader;
    shader_stage_create_info[1].pName  = "main";

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};

    vertex_input_info.sType                           =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 0;
    vertex_input_info.pVertexBindingDescriptions      = NULL;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions    = NULL;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};

    input_assembly.sType                  =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    /* viewport */
    VkViewport viewport = {0};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)meta->swap_chain_extent.width;
    viewport.height   = (float)meta->swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    /* scissor */
    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = meta->swap_chain_extent;

    VkPipelineViewportStateCreateInfo viewport_state = {0};

    viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = &viewport;
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = &scissor;

    /* rasterizer */
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};

    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    /* multisampling */
    VkPipelineMultisampleStateCreateInfo multisampling = {0};

    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    /* color blending */
    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};

    color_blend_attachment.colorWriteMask      =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {0};

    color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};

    pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 0;
    pipeline_layout_info.pSetLayouts            = NULL;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges    = NULL;

    if ( vkCreatePipelineLayout(meta->logical_device, &pipeline_layout_info,
                NULL, &meta->pipeline_layout ) != VK_SUCCESS )
    {
        gfx->msg = "pipeline layout konnte nicht erstellt werden";

        return GFX_FAILURE;
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {0};

    pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount          = 2;
    pipeline_info.pStages             = shader_stage_create_info;
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pDepthStencilState  = NULL;
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = NULL;
    pipeline_info.layout              = meta->pipeline_layout;
    pipeline_info.renderPass          = meta->render_pass;
    pipeline_info.subpass             = 0;
    pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex   = -1;

    if ( vkCreateGraphicsPipelines(meta->logical_device, VK_NULL_HANDLE,
                1, &pipeline_info, NULL, &meta->pipeline) != VK_SUCCESS )
    {
        gfx->msg = "graphics pipeline konnte nicht erstellt werden";

        return GFX_FAILURE;
    }

    /* aufräumen */
    vkDestroyShaderModule(meta->logical_device, vert_shader, NULL);
    vkDestroyShaderModule(meta->logical_device, frag_shader, NULL);
    /* }}} */
    /* framebuffer {{{ */
    meta->num_framebuffers = meta->num_image_views;
    meta->framebuffers     = ALLOC_SIZE(gfx->perm_arena, sizeof(VkFramebuffer)*meta->num_framebuffers);

    for ( uint32_t i = 0; i < meta->num_image_views; ++i ) {
        VkImageView attachments[] = {
            meta->image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_info = {0};

        framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass      = meta->render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = attachments;
        framebuffer_info.width           = meta->swap_chain_extent.width;
        framebuffer_info.height          = meta->swap_chain_extent.height;
        framebuffer_info.layers          = 1;

        if ( vkCreateFramebuffer(meta->logical_device, &framebuffer_info,
                    NULL, meta->framebuffers + i) != VK_SUCCESS )
        {
            gfx->msg = "framebuffer konnte nicht erstellt werden";

            return GFX_FAILURE;
        }
    }
    /* }}} */
    /* command pool {{{ */
    VkCommandPoolCreateInfo pool_info = {0};

    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = queue_index.graphics;
    pool_info.flags            = 0;

    if ( vkCreateCommandPool(meta->logical_device, &pool_info,
                NULL, &meta->command_pool) != VK_SUCCESS )
    {
        gfx->msg = "command pool konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* command buffers {{{ */
    meta->num_command_buffers = meta->num_framebuffers;
    meta->command_buffers = ALLOC_SIZE(gfx->perm_arena,
            sizeof(VkCommandBuffer)*meta->num_command_buffers);

    VkCommandBufferAllocateInfo command_buffer_info = {0};

    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.commandPool        = meta->command_pool;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_info.commandBufferCount = meta->num_command_buffers;

    if ( vkAllocateCommandBuffers(meta->logical_device, &command_buffer_info,
                meta->command_buffers) != VK_SUCCESS )
    {
        gfx->msg = "command buffer konnten nicht erstellt werden";

        return GFX_FAILURE;
    }

    for ( uint32_t i = 0; i < meta->num_command_buffers; ++i ) {
        VkCommandBufferBeginInfo begin_info = {0};

        begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags            = 0;
        begin_info.pInheritanceInfo = NULL;

        if ( vkBeginCommandBuffer(meta->command_buffers[i], &begin_info) != VK_SUCCESS ) {
            gfx->msg = "aufnahmen eines command buffer konnte nicht gestartet werden";

            return GFX_FAILURE;
        }

        VkClearValue clear_color = (VkClearValue){0.0f, 0.0f, 0.0f, 1.0f};
        VkRenderPassBeginInfo render_pass_info_i = {0};

        render_pass_info_i.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info_i.renderPass        = meta->render_pass;
        render_pass_info_i.framebuffer       = meta->framebuffers[i];
        render_pass_info_i.renderArea.offset = (VkOffset2D){0, 0};
        render_pass_info_i.renderArea.extent = meta->swap_chain_extent;
        render_pass_info_i.clearValueCount   = 1;
        render_pass_info_i.pClearValues      = &clear_color;

        vkCmdBeginRenderPass(meta->command_buffers[i], &render_pass_info_i, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(meta->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, meta->pipeline);
        vkCmdDraw(meta->command_buffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(meta->command_buffers[i]);

        if ( vkEndCommandBuffer(meta->command_buffers[i]) != VK_SUCCESS ) {
            gfx->msg = "command buffer konnten abgeschlossen werden";

            return GFX_FAILURE;
        }
    }
    /* }}} */
    /* semaphoren {{{ */
    VkSemaphoreCreateInfo semaphore_info = {0};

    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if ( vkCreateSemaphore(meta->logical_device, &semaphore_info, NULL,
                &meta->image_available_semaphore) != VK_SUCCESS         ||
         vkCreateSemaphore(meta->logical_device, &semaphore_info, NULL,
               &meta->render_finished_semaphore)  != VK_SUCCESS )
    {
        gfx->msg = "semaphoren konnten nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */

    return GFX_SUCCESS;
}

GFX_API_SHADER() {
    Gfx_Shader result = {0};
    VkShaderModule shader = {0};

    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    char *shader_code = "";
    size_t shader_size = 0;
    if ( !os_readfile(filename, &shader_code, &shader_size) ) {
        return result;
    }

    VkShaderModuleCreateInfo create_info = {0};

    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_size;
    create_info.pCode    = (uint32_t *)shader_code;

    result.meta = ALLOC_SIZE(arena, sizeof(VkShaderModule));
    if ( vkCreateShaderModule(meta->logical_device, &create_info, NULL, result.meta) != VK_SUCCESS ) {
        return result;
    }

    return result;
}

GFX_API_CLEANUP() {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    vkDestroySemaphore(meta->logical_device, meta->render_finished_semaphore, NULL);
    vkDestroySemaphore(meta->logical_device, meta->image_available_semaphore, NULL);
    vkDestroyCommandPool(meta->logical_device, meta->command_pool, NULL);
    vkDestroyPipeline(meta->logical_device, meta->pipeline, NULL);
    vkDestroyPipelineLayout(meta->logical_device, meta->pipeline_layout, NULL);

    for ( uint32_t framebuffer_idx = 0; framebuffer_idx < meta->num_framebuffers; ++framebuffer_idx ) {
        vkDestroyFramebuffer(meta->logical_device, meta->framebuffers[framebuffer_idx], NULL);
    }

    vkDestroyRenderPass(meta->logical_device, meta->render_pass, NULL);

    for ( uint32_t image_view_idx = 0; image_view_idx < meta->num_image_views; ++image_view_idx ) {
        vkDestroyImageView(meta->logical_device, meta->image_views[image_view_idx], NULL);
    }

    vkDestroySwapchainKHR(meta->logical_device, meta->swap_chain, NULL);
    vkDestroySurfaceKHR(meta->instance, meta->surface, NULL);
    vkDestroyDevice(meta->logical_device, NULL);
    vkDestroyInstance(meta->instance, NULL);
}

GFX_API_RENDER() {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    uint32_t image_index;
    vkAcquireNextImageKHR(meta->logical_device, meta->swap_chain, UINT64_MAX,
            meta->image_available_semaphore, VK_NULL_HANDLE, &image_index);

    VkSemaphore wait_semaphores[]      = { meta->image_available_semaphore };
    VkSemaphore signal_semaphores[]    = { meta->render_finished_semaphore };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit_info           = {0};

    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = wait_semaphores;
    submit_info.pWaitDstStageMask    = wait_stages;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = meta->command_buffers + image_index;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    if ( vkQueueSubmit( meta->gfx_queue, 1, &submit_info, VK_NULL_HANDLE ) != VK_SUCCESS ) {
        gfx->msg = "fehler beim absetzen der grafischen warteschlange";
    }

    VkSwapchainKHR swap_chains[] = { meta->swap_chain };
    VkPresentInfoKHR present_info = {0};

    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = swap_chains;
    present_info.pImageIndices      = &image_index;
    present_info.pResults           = NULL;

    vkQueuePresentKHR(meta->present_queue, &present_info);

#if 0
    for ( uint32_t i = 0; i < queue->num_cmds; ++i ) {
        Gfx_Command *cmd = queue->cmds[i];

        switch ( cmd->kind ) {
            case GFX_COMMAND_RECT: {
                gfx_rect(cmd->rect.dim, cmd->rect.c);
            } break;
        }
    }
#endif
}

GFX_API_RECT() {
}
/* }}} */

