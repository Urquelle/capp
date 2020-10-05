#define ARRAY_SIZE(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))
#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))
#define CLAMP_MAX(x, max) MIN(x, max)
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))

typedef struct Pos           Pos;
typedef struct Rect          Rect;
typedef struct Color         Color;

struct Pos {
    float x;
    float y;
};

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

struct Color {
    union {
        uint32_t rgba;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        } elems;
    };
};

Color COLOR_WHITE = { .rgba = 0xffff };
Color COLOR_BLACK = { .rgba = 0x0000 };

Color
color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Color result = {0};

    result.elems.r = r;
    result.elems.g = g;
    result.elems.b = b;
    result.elems.a = a;

    return result;
}

Rect
rect(float x, float y, float width, float height) {
    Rect result = {0};

    result.x = x;
    result.y = y;
    result.width = width;
    result.height = height;

    return result;
}

bool
pos_inside_rect(Rect rect, Pos pos) {
    bool result = false;

    if ( pos.x >= rect.x && pos.x <= (rect.x+rect.width) &&
         pos.y >= rect.y && pos.y <= (rect.y+rect.height) )
    {
        result = true;
    }

    return result;
}

