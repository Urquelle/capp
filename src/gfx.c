typedef struct Gfx           Gfx;
typedef struct Gfx_Command   Gfx_Command;
typedef struct Gfx_Queue     Gfx_Queue;
typedef struct Gfx_Shader    Gfx_Shader;
typedef struct Gfx_Program   Gfx_Program;
typedef struct Gfx_Texture   Gfx_Texture;
typedef struct Gfx_Shader    Gfx_Shader;
typedef struct Gfx_Vertex    Gfx_Vertex;
typedef enum   Gfx_Result    Gfx_Result;

enum Gfx_Result {
    GFX_FALSE,
    GFX_FAILURE = GFX_FALSE,
    GFX_TRUE,
    GFX_SUCCESS = GFX_TRUE,
};

enum Gfx_Command_Kind {
    GFX_COMMAND_NONE,
    GFX_COMMAND_RECT,
};
struct Gfx_Command {
    uint32_t kind;

    union {
        struct {
            Rect dim;
            Color c;
        } rect;
    };
};

struct Gfx_Queue {
    uint32_t        num_cmds;
    Gfx_Command **  cmds;
};

struct Gfx_Vertex {
    Vec2 pos;
    Vec3 color;
};

struct Gfx_Shader {
    void * meta;
};

struct Gfx_Program {
    Gfx_Shader vert_shader;
    Gfx_Shader frag_shader;
};

struct Gfx {
    Gfx_Queue   queue;
    Mem_Arena * perm_arena;
    Mem_Arena * temp_arena;
    char      * msg;
    void      * meta;
};

/* ################################################### */
/* ####################### API ####################### */
/* ################################################### */

#define GFX_API_INIT()    Gfx_Result     gfx_init_impl(Gfx *gfx, Os *os)
#define GFX_API_RESIZE()  Gfx_Result     gfx_resize_impl(Gfx *gfx, Os *os)
#define GFX_API_RENDER()  Gfx_Result     gfx_render_impl(Gfx *gfx, Os *os, Gfx_Queue *queue)
#define GFX_API_CLEANUP() void           gfx_cleanup_impl(Gfx *gfx)
#define GFX_API_SHADER()  Gfx_Shader     gfx_shader(Gfx *gfx, char *filename, Mem_Arena *arena)
#define GFX_API_PROGRAM() Gfx_Program    gfx_program(Gfx *gfx)
#define GFX_API_RECT()    void           gfx_rect(Rect rect, Color color)

GFX_API_INIT();
GFX_API_RESIZE();
GFX_API_RENDER();
GFX_API_CLEANUP();
GFX_API_SHADER();
GFX_API_PROGRAM();
GFX_API_RECT();

/* ################################################## */

#include "gfx_vulkan.c"

Gfx_Result
gfx_init(Gfx *gfx, Os *os, Mem_Arena *perm_arena, Mem_Arena *temp_arena) {
    gfx->perm_arena = perm_arena;
    gfx->temp_arena = temp_arena;

    Gfx_Result result = gfx_init_impl(gfx, os);

    return result;
}

Gfx_Result
gfx_resize(Gfx *gfx, Os *os) {
    uint32_t result = gfx_resize_impl(gfx, os);

    return result;
}

void
gfx_push(Gfx *gfx, Gfx_Command *cmd) {
    buf_push(gfx->queue.cmds, cmd);
    gfx->queue.num_cmds += 1;
}

Gfx_Result
gfx_render(Gfx *gfx, Os *os, Gfx_Queue *queue) {
    Gfx_Result result = gfx_render_impl(gfx, os, queue);
    buf_free(queue->cmds);
    queue->num_cmds = 0;

    return result;
}

void
gfx_cleanup(Gfx *gfx) {
    gfx_cleanup_impl(gfx);
    mem_reset(gfx->perm_arena);
}

Gfx_Command *
gfx_cmd_rect(Rect dim, Color c, Mem_Arena *arena) {
    Gfx_Command *result = ALLOC_STRUCT(arena, Gfx_Command);

    result->kind = GFX_COMMAND_RECT;
    result->rect.dim = dim;
    result->rect.c = c;

    return result;
}

