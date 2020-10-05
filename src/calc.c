typedef struct Vec2          Vec2;
typedef struct Vec2i         Vec2i;
typedef struct Vec3          Vec3;

struct Vec2 {
    union {
        struct {
            float x;
            float y;
        };

        struct {
            float width;
            float height;
        };

        struct {
            float min;
            float max;
        };
    };
};

struct Vec2i {
    union {
        struct {
            int32_t x;
            int32_t y;
        };

        struct {
            int32_t width;
            int32_t height;
        };

        struct {
            int32_t min;
            int32_t max;
        };
    };
};

struct Vec3 {
    union {
        struct {
            float x;
            float y;
            float z;
        };

        struct {
            float r;
            float g;
            float b;
        };
    };
};
