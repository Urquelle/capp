#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.c"
#include "mem.c"
#include "utf8.c"
#include "util.c"
#include "os.c"
#include "gfx.c"
#include "ui.c"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {
    Mem_Arena *perm_arena = &(Mem_Arena){0};
    mem_arena_init(perm_arena);
    Mem_Arena *temp_arena = &(Mem_Arena){0};
    mem_arena_init(temp_arena);

    Os *os = &(Os){0};
    if ( !os_init(os) ) {
        exit(1);
    }

    Gfx *gfx = &(Gfx){0};
    if ( !gfx_init(gfx, os, temp_arena) ) {
        exit(1);
    }

    Ui *ui = &(Ui){0};
    if ( !ui_init(ui, &os->mouse, gfx, perm_arena) ) {
        exit(1);
    }

    while ( !os->quit ) {
        os_pull(os);

        if ( ui_btn(ui, 1, "drück mich ... ganz fest!") ) {
            printf("FEST, hab ich gesagt!\n");
        }

        os_push(os);

        gfx_render(gfx, &gfx->queue);
        gfx_reset(gfx);
    }

    return 0;
}
