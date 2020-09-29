typedef struct Gfx           Gfx;
typedef struct Gfx_Command   Gfx_Command;
typedef struct Gfx_Queue     Gfx_Queue;
typedef struct Gfx_Texture   Gfx_Texture;
typedef struct Gfx_Shader    Gfx_Shader;

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

struct Gfx {
    Gfx_Queue   queue;
    Mem_Arena * arena;
    void      * meta;
};

#include "gfx_vulkan.c"

Gfx_Command *
gfx_cmd_rect(Rect dim, Color c, Mem_Arena *arena) {
    Gfx_Command *result = ALLOC_STRUCT(arena, Gfx_Command);

    result->kind = GFX_COMMAND_RECT;
    result->rect.dim = dim;
    result->rect.c = c;

    return result;
}

void
gfx_push(Gfx *gfx, Gfx_Command *cmd) {
    buf_push(gfx->queue.cmds, cmd);
    gfx->queue.num_cmds += 1;
}

void
gfx_render(Gfx *gfx, Gfx_Queue *queue) {
    for ( uint32_t i = 0; i < queue->num_cmds; ++i ) {
        Gfx_Command *cmd = queue->cmds[i];

        switch ( cmd->kind ) {
            case GFX_COMMAND_RECT: {
                gfx_rect(cmd->rect.dim, cmd->rect.c);
            } break;
        }
    }
}

void
gfx_reset(Gfx *gfx) {
    free(gfx->queue.cmds);
    gfx->queue.num_cmds = 0;
    mem_reset(gfx->arena);
}

