#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

void gfx_vulkan_cleanup_swapchain(Gfx *gfx);

enum { GFX_MAX_FRAMES_IN_FLIGHT = 2 };
typedef struct Gfx_Vulkan_Meta  Gfx_Vulkan_Meta;

struct Gfx_Vulkan_Meta {
    VkInstance                instance;
    VkDebugUtilsMessengerEXT  debug_messenger;
    VkPhysicalDevice          physical_device;
    VkDevice                  logical_device;
    VkQueue                   gfx_queue;
    VkQueue                   present_queue;

    struct {
        int32_t graphics;
        int32_t present;
    }                         queue_index;

    VkSurfaceKHR              surface;
    VkSurfaceFormatKHR        format;
    VkPresentModeKHR          present_mode;
    VkExtent2D                swap_chain_extent;
    VkSwapchainKHR            swap_chain;
    VkImage               *   swap_chain_images;
    uint32_t                  num_swap_chain_images;
    VkImageView           *   image_views;
    uint32_t                  num_image_views;
    VkFramebuffer         *   framebuffers;
    uint32_t                  num_framebuffers;
    VkRenderPass              render_pass;
    VkPipelineLayout          pipeline_layout;
    VkPipeline                pipeline;
    VkCommandPool             command_pool;
    VkCommandBuffer       *   command_buffers;
    uint32_t                  num_command_buffers;
    VkSemaphore               image_available_semaphore[GFX_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore               render_finished_semaphore[GFX_MAX_FRAMES_IN_FLIGHT];
    VkFence                   in_flight_fences[GFX_MAX_FRAMES_IN_FLIGHT];
    VkFence               *   images_in_flight;
    uint32_t                  num_images_in_flight;
    VkBuffer                  vertex_buffer;
    VkDeviceMemory            vertex_buffer_memory;
    size_t                    current_frame;
};

typedef struct {
    VkSurfaceCapabilitiesKHR    capabilities;
    uint32_t                    num_formats;
    VkSurfaceFormatKHR *        formats;
    uint32_t                    num_present_modes;
    VkPresentModeKHR   *        present_modes;
} Gfx_Vulkan_Swap_Chain_Details;

typedef struct {
    VkVertexInputAttributeDescription description[2];
} Gfx_Vulkan_Attribute_Description;

char *required_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
uint32_t num_required_device_extensions = ARRAY_SIZE(required_device_extensions);

Gfx_Vertex vertices[] = {
    {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
};

VKAPI_ATTR VkBool32 VKAPI_CALL gfx_vulkan_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void * user_data)
{
    if ( message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) {
    }

    return VK_FALSE;
}

/* vulkan hilfsmethoden {{{ */
Gfx_Vulkan_Attribute_Description
gfx_vulkan_attribute_descriptions() {
    Gfx_Vulkan_Attribute_Description result = {0};

    result.description[0].binding  = 0;
    result.description[0].location = 0;
    result.description[0].format   = VK_FORMAT_R32G32_SFLOAT;
    result.description[0].offset   = offsetof(Gfx_Vertex, pos);

    result.description[1].binding  = 0;
    result.description[1].location = 1;
    result.description[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    result.description[1].offset   = offsetof(Gfx_Vertex, color);

    return result;
}

Gfx_Result
gfx_vulkan_create_instance(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    /* app info {{{ */
    VkApplicationInfo app_info = {0};

    app_info.sType                   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName        = "HelloTriangle";
    app_info.applicationVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName             = "No Engine";
    app_info.engineVersion           = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion              = VK_API_VERSION_1_0;
    /* }}} */
    /* erweiterungen {{{ */
    char *required_extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
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
    /* instanz {{{ */
    VkDebugUtilsMessengerCreateInfoEXT debug_info = {0};

    debug_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType     =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = gfx_vulkan_debug_callback;
    debug_info.pUserData       = NULL;

    VkInstanceCreateInfo inst_info = {0};

    inst_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pApplicationInfo        = &app_info;
    inst_info.enabledExtensionCount   = num_required_extensions;
    inst_info.ppEnabledExtensionNames = required_extensions;

    if ( validation_supported ) {
        inst_info.enabledLayerCount = ARRAY_SIZE(validation_layers);
        inst_info.ppEnabledLayerNames = validation_layers;
        inst_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_info;
    } else {
        inst_info.enabledLayerCount = 0;
        inst_info.pNext = NULL;
    }

    if ( vkCreateInstance(&inst_info, NULL, &meta->instance) != VK_SUCCESS ) {
        gfx->msg = "vulkan instanz konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* debug messenger {{{ */
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(meta->instance, "vkCreateDebugUtilsMessengerEXT");

    if ( !func ) {
        gfx->msg = "debug erstellungsmethode konnte nicht ermittelt werden";

        return GFX_FAILURE;
    }

    if ( func(meta->instance, &debug_info, NULL, &meta->debug_messenger) != VK_SUCCESS ) {
        gfx->msg = "debug messenger konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_create_surface(Gfx *gfx, Os *os) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;
    Os_Win32_Meta *os_meta = (Os_Win32_Meta *)os->meta;

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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_image_views(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_render_pass(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_graphics_pipeline(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

    Gfx_Vulkan_Attribute_Description vertex_attributes = gfx_vulkan_attribute_descriptions();

    VkVertexInputBindingDescription binding_description = {0};

    binding_description.binding   = 0;
    binding_description.stride    = sizeof(Gfx_Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};

    vertex_input_info.sType                           =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 1;
    vertex_input_info.pVertexBindingDescriptions      = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = ARRAY_SIZE(vertex_attributes.description);
    vertex_input_info.pVertexAttributeDescriptions    = vertex_attributes.description;

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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_framebuffer(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_command_pool(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    /* command pool {{{ */
    VkCommandPoolCreateInfo pool_info = {0};

    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = meta->queue_index.graphics;
    pool_info.flags            = 0;

    if ( vkCreateCommandPool(meta->logical_device, &pool_info,
                NULL, &meta->command_pool) != VK_SUCCESS )
    {
        gfx->msg = "command pool konnte nicht erstellt werden";

        return GFX_FAILURE;
    }
    /* }}} */

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_vertex_buffer(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    /* vertex buffer {{{ */
    VkBufferCreateInfo buffer_info = {0};

    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = sizeof(vertices[0])*ARRAY_SIZE(vertices);
    buffer_info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if ( vkCreateBuffer(meta->logical_device, &buffer_info,
                NULL, &meta->vertex_buffer) != VK_SUCCESS )
    {
        gfx->msg = "vertex buffer konnten nicht erstellt werden";

        return GFX_FAILURE;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(meta->logical_device, meta->vertex_buffer, &mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(meta->physical_device, &mem_properties);

    /* passenden speichertyp finden {{{ */
    uint32_t              type_filter   = mem_requirements.memoryTypeBits;
    VkMemoryPropertyFlags properties    = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    int32_t mem_index = -1;
    for ( uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i ) {
        if ( (type_filter & ( 1 << i )) &&
             (mem_properties.memoryTypes[i].propertyFlags & properties) == properties )
        {
            mem_index = i;
            break;
        }
    }

    if ( mem_index == -1 ) {
        gfx->msg = "kein passender speicher konnte für vertex buffer ermittelt werden";

        return GFX_FAILURE;
    }
    /* }}} */

    VkMemoryAllocateInfo alloc_info = {0};

    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_requirements.size;
    alloc_info.memoryTypeIndex = mem_index;

    if ( vkAllocateMemory(meta->logical_device, &alloc_info,
                NULL, &meta->vertex_buffer_memory ) != VK_SUCCESS )
    {
        gfx->msg = "speicher für vertex buffer konnte nicht reserviert werden";

        return GFX_FAILURE;
    }

    vkBindBufferMemory(meta->logical_device, meta->vertex_buffer, meta->vertex_buffer_memory, 0);

    void *data;
    vkMapMemory(meta->logical_device, meta->vertex_buffer_memory, 0, buffer_info.size, 0, &data);
    memcpy(data, vertices, buffer_info.size);
    vkUnmapMemory(meta->logical_device, meta->vertex_buffer_memory);
    /* }}} */

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_command_buffers(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

        VkBuffer vertex_buffers[] = { meta->vertex_buffer };
        VkDeviceSize offsets[]    = {0};
        vkCmdBindVertexBuffers(meta->command_buffers[i], 0, 1, vertex_buffers, offsets);

        vkCmdDraw(meta->command_buffers[i], ARRAY_SIZE(vertices), 1, 0, 0);
        vkCmdEndRenderPass(meta->command_buffers[i]);

        if ( vkEndCommandBuffer(meta->command_buffers[i]) != VK_SUCCESS ) {
            gfx->msg = "command buffer konnten abgeschlossen werden";

            return GFX_FAILURE;
        }
    }
    /* }}} */

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_sync_objects(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    /* semaphoren {{{ */
    VkSemaphoreCreateInfo semaphore_info = {0};

    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for ( uint32_t i = 0; i < GFX_MAX_FRAMES_IN_FLIGHT; ++i ) {
        if ( vkCreateSemaphore(meta->logical_device, &semaphore_info, NULL,
                    &meta->image_available_semaphore[i]) != VK_SUCCESS
             ||
             vkCreateSemaphore(meta->logical_device, &semaphore_info, NULL,
                    &meta->render_finished_semaphore[i])  != VK_SUCCESS )
        {
            gfx->msg = "semaphoren konnten nicht erstellt werden";

            return GFX_FAILURE;
        }
    }
    /* }}} */
    /* fences {{{ */
    meta->num_images_in_flight = meta->num_swap_chain_images;
    meta->images_in_flight     = ALLOC_SIZE(gfx->perm_arena, sizeof(VkFence)*meta->num_images_in_flight);

    for ( uint32_t i = 0; i < meta->num_images_in_flight; ++i ) {
        meta->images_in_flight[i] = (VkFence){ VK_NULL_HANDLE };
    }

    VkFenceCreateInfo fence_info = {0};

    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( uint32_t i = 0; i < GFX_MAX_FRAMES_IN_FLIGHT; ++i ) {
        if ( vkCreateFence(meta->logical_device, &fence_info, NULL,
                    &meta->in_flight_fences[i]) != VK_SUCCESS )
        {
            gfx->msg = "fences konnten nicht erstellt werden";

            return GFX_FAILURE;
        }
    }
    /* }}} */

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_physical_device(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_logical_device(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    /* auftragsschlangen {{{ */
    uint32_t num_queues_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(meta->physical_device, &num_queues_families, NULL);
    VkQueueFamilyProperties *queue_families = ALLOC_SIZE(gfx->temp_arena, sizeof(VkQueueFamilyProperties)*num_queues_families);
    vkGetPhysicalDeviceQueueFamilyProperties(meta->physical_device, &num_queues_families, queue_families);

    meta->queue_index.graphics = -1;
    meta->queue_index.present  = -1;

    VkBool32 present_support = false;
    for ( uint32_t queue_idx = 0; queue_idx < num_queues_families; ++queue_idx ) {
        VkQueueFamilyProperties queue_family = queue_families[queue_idx];
        if ( queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
            meta->queue_index.graphics = (int32_t)queue_idx;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(meta->physical_device, queue_idx, meta->surface, &present_support);
        if ( present_support && meta->queue_index.present == -1 &&
                meta->queue_index.present != meta->queue_index.graphics )
        {
            meta->queue_index.present = (int32_t)queue_idx;
        }
    }

    if ( meta->queue_index.graphics == -1 || meta->queue_index.present == -1 ) {
        gfx->msg = "notwendigen warteschlangen konnten nicht ermittelt werden";

        return GFX_FAILURE;
    }
    /* }}} */
    /* logisches gerät {{{ */
    VkDeviceQueueCreateInfo queue_info[2] = {0};
    float queue_prio = 1.0f;

    /* gfx queue {{{ */
    queue_info[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = meta->queue_index.graphics;
    queue_info[0].queueCount       = 1;
    queue_info[0].pQueuePriorities = &queue_prio;
    /* }}} */
    /* present queue {{{ */
    queue_info[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[1].queueFamilyIndex = meta->queue_index.present;
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

    vkGetDeviceQueue(meta->logical_device, meta->queue_index.graphics, 0, &meta->gfx_queue);
    vkGetDeviceQueue(meta->logical_device, meta->queue_index.present,  0, &meta->present_queue);
    /* }}} */

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_init_swapchain(Gfx *gfx, Os *os) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

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

    if ( meta->queue_index.graphics != meta->queue_index.present ) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        swap_chain_create_info.pQueueFamilyIndices = (uint32_t[]){
            meta->queue_index.graphics,
            meta->queue_index.present
        };
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

    return GFX_SUCCESS;
}

Gfx_Result
gfx_vulkan_reinit_swapchain(Gfx *gfx, Os *os) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    vkDeviceWaitIdle(meta->logical_device);
    gfx_vulkan_cleanup_swapchain(gfx);

    if ( gfx_vulkan_init_swapchain(gfx, os)     != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_image_views(gfx)       != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_render_pass(gfx)       != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_graphics_pipeline(gfx) != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_framebuffer(gfx)       != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_command_buffers(gfx)   != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    return GFX_SUCCESS;
}

void
gfx_vulkan_cleanup_swapchain(Gfx *gfx) {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    for ( uint32_t framebuffer_idx = 0; framebuffer_idx < meta->num_framebuffers; ++framebuffer_idx ) {
        vkDestroyFramebuffer(meta->logical_device, meta->framebuffers[framebuffer_idx], NULL);
    }

    vkFreeCommandBuffers(meta->logical_device, meta->command_pool, meta->num_command_buffers,
            meta->command_buffers);

    vkDestroyPipeline(meta->logical_device, meta->pipeline, NULL);
    vkDestroyPipelineLayout(meta->logical_device, meta->pipeline_layout, NULL);
    vkDestroyRenderPass(meta->logical_device, meta->render_pass, NULL);

    for ( uint32_t image_view_idx = 0; image_view_idx < meta->num_image_views; ++image_view_idx ) {
        vkDestroyImageView(meta->logical_device, meta->image_views[image_view_idx], NULL);
    }

    vkDestroySwapchainKHR(meta->logical_device, meta->swap_chain, NULL);
}
/* }}} */
/* gfx api {{{ */
/* init {{{ */
GFX_API_INIT() {
    Gfx_Vulkan_Meta *meta = ALLOC_STRUCT(gfx->perm_arena, Gfx_Vulkan_Meta);
    *meta = (Gfx_Vulkan_Meta){0};
    gfx->meta = meta;

    if ( gfx_vulkan_create_instance(gfx)        != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_create_surface(gfx, os)     != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_physical_device(gfx)   != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_logical_device(gfx)    != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_swapchain(gfx, os)     != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_image_views(gfx)       != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_render_pass(gfx)       != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_graphics_pipeline(gfx) != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_framebuffer(gfx)       != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_command_pool(gfx)      != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_vertex_buffer(gfx)     != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_command_buffers(gfx)   != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    if ( gfx_vulkan_init_sync_objects(gfx)      != GFX_SUCCESS ) {
        return GFX_FAILURE;
    }

    return GFX_SUCCESS;
}
/* }}} */
/* resize {{{ */
GFX_API_RESIZE() {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    vkDeviceWaitIdle(meta->logical_device);
    gfx_vulkan_cleanup_swapchain(gfx);
    uint32_t result = gfx_vulkan_reinit_swapchain(gfx, os);

    return result;
}
/* }}} */
/* shader {{{ */
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
/* }}} */
/* cleanup {{{ */
GFX_API_CLEANUP() {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    gfx_vulkan_cleanup_swapchain(gfx);

    vkDestroyBuffer(meta->logical_device, meta->vertex_buffer, NULL);
    vkFreeMemory(meta->logical_device, meta->vertex_buffer_memory, NULL);

    for ( uint32_t i = 0; i < GFX_MAX_FRAMES_IN_FLIGHT; ++i ) {
        vkDestroySemaphore(meta->logical_device, meta->render_finished_semaphore[i], NULL);
        vkDestroySemaphore(meta->logical_device, meta->image_available_semaphore[i], NULL);
        vkDestroyFence(meta->logical_device, meta->in_flight_fences[i], NULL);
    }

    vkDestroyCommandPool(meta->logical_device, meta->command_pool, NULL);
    vkDestroyDevice(meta->logical_device, NULL);
    vkDestroySurfaceKHR(meta->instance, meta->surface, NULL);

    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(meta->instance, "vkDestroyDebugUtilsMessengerEXT");
    if ( func ) {
        func(meta->instance, meta->debug_messenger, NULL);
    }

    vkDestroyInstance(meta->instance, NULL);
}
/* }}} */
/* render {{{ */
GFX_API_RENDER() {
    Gfx_Vulkan_Meta *meta = (Gfx_Vulkan_Meta *)gfx->meta;

    vkWaitForFences(meta->logical_device, 1, &meta->in_flight_fences[meta->current_frame], VK_TRUE,
            UINT64_MAX);

    uint32_t image_index;
    VkResult ret = vkAcquireNextImageKHR(meta->logical_device, meta->swap_chain, UINT64_MAX,
            meta->image_available_semaphore[meta->current_frame], VK_NULL_HANDLE, &image_index);

    if ( ret == VK_ERROR_OUT_OF_DATE_KHR ) {
        gfx_vulkan_init_swapchain(gfx, os);
        os->window.dim_changed = false;

        return GFX_SUCCESS;
    } else if ( ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR ) {
        gfx->msg = "konnte kein bild aus der swap chain anfordern";

        return GFX_FAILURE;
    }

    if ( meta->images_in_flight[image_index] != VK_NULL_HANDLE ) {
        vkWaitForFences(meta->logical_device, 1, &meta->images_in_flight[image_index],
                VK_TRUE, UINT64_MAX);
    }

    meta->images_in_flight[image_index] = meta->in_flight_fences[meta->current_frame];

    VkSemaphore wait_semaphores[]      = { meta->image_available_semaphore[meta->current_frame] };
    VkSemaphore signal_semaphores[]    = { meta->render_finished_semaphore[meta->current_frame] };
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

    vkResetFences(meta->logical_device, 1, &meta->in_flight_fences[meta->current_frame]);

    if ( vkQueueSubmit( meta->gfx_queue, 1, &submit_info,
                meta->in_flight_fences[meta->current_frame] ) != VK_SUCCESS )
    {
        gfx->msg = "fehler beim absetzen der grafischen warteschlange";

        return GFX_FAILURE;
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

    meta->current_frame = (meta->current_frame + 1) % GFX_MAX_FRAMES_IN_FLIGHT;

    return GFX_SUCCESS;

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
/* }}} */
/* rect {{{ */
GFX_API_RECT() {
}
/* }}} */
/* }}} */

