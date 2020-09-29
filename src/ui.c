typedef struct Ui           Ui;
typedef struct Ui_Btn       Ui_Btn;
typedef struct Ui_Layout    Ui_Layout;
typedef enum   Ui_Align     Ui_Align;

enum Ui_Align {
    UI_DEFAULT,
    UI_LEFT,
    UI_RIGHT,
    UI_CENTER,
};

struct Ui {
    Map elems;

    uint32_t active_id;
    uint32_t hot_id;

    Os_Mouse *mouse;
    Gfx *gfx;
    Mem_Arena *arena;
};

struct Ui_Layout {
    uint32_t align;

    Pos base;
    Pos curr;
};

struct Ui_Btn {
    uint32_t  id;
    char *    txt;
};

Ui_Btn *
ui_btn_new(uint32_t id, char *txt, Mem_Arena *arena) {
    Ui_Btn *result = ALLOC_STRUCT(arena, Ui_Btn);

    result->id   = id;
    result->txt  = txt;

    return result;
}

bool
ui_init(Ui *ui, Os_Mouse *mouse, Gfx *gfx, Mem_Arena *arena) {
    ui->mouse = mouse;
    ui->gfx   = gfx;
    ui->arena = arena;

    return true;
}

void *
ui_elem_find(Ui *ui, uint32_t id) {
    void *result = map_get(&ui->elems, (void *)(uintptr_t)id);

    return result;
}

void
ui_elem_push(Ui *ui, uint32_t id, void *elem) {
    map_push(&ui->elems, (void *)(uintptr_t)id, elem, ui->arena);
}

bool
ui_btn(Ui *ui, uint32_t id, char *txt) {
    Ui_Btn *btn = (Ui_Btn *)ui_elem_find(ui, id);

    Rect r = rect(10, 10, 300, 50);

    if ( !btn ) {
        btn = ui_btn_new(id, txt, ui->arena);
        ui_elem_push(ui, id, btn);
    }

    if (ui->active_id == id) {
        if (ui->mouse->left.released) {
            if (ui->hot_id == id) {
                return true;
            }

            ui->active_id = 0;
        }
    } else if (ui->hot_id == id) {
        if (ui->mouse->left.pressed) {
            ui->active_id = id;
        }
    }

    if ( pos_inside_rect(r, ui->mouse->pos) ) {
        ui->hot_id = id;
    } else {
        if (ui->hot_id == id) {
            ui->hot_id = 0;
        }
    }

    gfx_push(ui->gfx, gfx_cmd_rect(r, COLOR_BLACK, ui->gfx->arena));

    return false;
}

