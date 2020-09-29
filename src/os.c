typedef struct Os_Button Os_Button;
typedef struct Os_Mouse  Os_Mouse;
typedef struct Os        Os;

struct Os_Button {
    bool down;
    bool pressed;
    bool released;
    bool lowercase;
    bool uppercase;
};

struct Os_Mouse {
    Pos pos;
    Pos dpos;

    Os_Button left;
    Os_Button right;
    Os_Button middle;

    int32_t delta_wheel;
};

struct Os {
    bool quit;

    char chars[11];
    int num_chars;

    Os_Mouse mouse;

    struct {
        Pos pos;
    } window;

    struct {
        char *   device;
        char *   version;
        char *   vendor;
    } graphics;

    void *meta;
};

#include "os_win32.c"

