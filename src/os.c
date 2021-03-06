typedef struct Os        Os;
typedef struct Os_Button Os_Button;
typedef struct Os_Mouse  Os_Mouse;
typedef struct Os_Timer  Os_Timer;
typedef enum   Os_Result Os_Result;

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

struct Os_Timer {
    uint64_t start;
    uint64_t end;

    struct {
        uint64_t microseconds;
        uint64_t milliseconds;
        float    seconds;
    } diff;
};

struct Os {
    bool quit;

    char chars[11];
    int num_chars;

    Os_Mouse mouse;

    struct {
        Pos      pos;
        Vec2i    dim;
        bool     dim_changed;
    } window;

    struct {
        char *   device;
        char *   version;
        char *   vendor;
    } graphics;

    struct {
        uint64_t delta_ticks;
        uint64_t delta_nanoseconds;
        uint64_t delta_microseconds;
        uint64_t delta_milliseconds;
        float    delta_seconds;

        uint64_t ticks;
        uint64_t nanoseconds;
        uint64_t microseconds;
        uint64_t milliseconds;
        float    seconds;

        uint64_t initial_ticks;
        uint64_t ticks_per_second;
    } time;

    char *msg;
    void *meta;
};

enum Os_Result {
    OS_FALSE,
    OS_FAILURE = OS_FALSE,
    OS_TRUE,
    OS_SUCCESS = OS_TRUE,
};

/* ################################################### */
/* ####################### API ####################### */
/* ################################################### */

#define OS_API_INIT()      Os_Result    os_init(Os *os)
#define OS_API_PULL()      void         os_pull(Os *os)
#define OS_API_PUSH()      void         os_push(Os *os)
#define OS_API_READFILE()  bool         os_readfile(char *filename, char **result, size_t *size)
#define OS_API_WRITEFILE() bool         os_writefile(char *filename, char *data, size_t len)
#define OS_API_DEBUG()     void         os_debug(char *msg)
#define OS_API_TIMER_START Os_Timer     os_timer_start()
#define OS_API_TIMER_STOP  void         os_timer_stop(Os_Timer *timer)

#include "os_win32.c"

