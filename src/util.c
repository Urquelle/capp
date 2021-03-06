#define buf__hdr(b) ((Buf_Hdr *)((char *)(b) - offsetof(Buf_Hdr, buf)))
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : (*((void **)&(b)) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

typedef struct Buf_Hdr          Buf_Hdr;
typedef struct Map              Map;
typedef struct Param_Pair       Param_Pair;
typedef struct Str_Intern       Str_Intern;

void map_push(Map *map, void *key, void *val, Mem_Arena *arena);

struct Map {
    size_t cap;
    size_t len;

    void **keys;
    void **vals;
};

struct Str_Intern {
    size_t length;
    Str_Intern * next;
    char str[1];
};

static uint64_t
util_bytes_hash(void *ptr, size_t len) {
    uint64_t x = 0xcbf29ce484222325;
    char *buf = (char *)ptr;

    for (size_t i = 0; i < len; i++) {
        x ^= buf[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }

    return x;
}

uint64_t util_ptr_hash(void *ptr) {
    uint64_t x = (uintptr_t)ptr;

    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;

    return x;
}

void map_grow(Map *map, Mem_Arena *arena) {
    size_t cap = ((map->cap * 2) < 16) ? 16 : map->cap * 2;
    void *mem = ALLOC_SIZE(arena, map->cap);

    Map new_map = {0};

    new_map.keys = ALLOC_SIZE(arena, cap*sizeof(void *));
    ZERO_ARRAY(cap, new_map.keys);
    new_map.vals = ALLOC_SIZE(arena, cap*sizeof(void *));

    for ( uint32_t i = 0; i < map->cap; ++i ) {
        if ( map->keys[i] ) {
            map_push(&new_map, map->keys[i], map->vals[i], arena);
        }
    }

    free(map->keys);
    free(map->vals);

    map->cap = cap;
    map->len = map->len;
    map->keys = new_map.keys;
    map->vals = new_map.vals;
}

void map_push(Map *map, void *key, void *val, Mem_Arena *arena) {
    if ( !map ) return;

    if ( (map->cap / 2) < (map->len + 1) ) {
        map_grow(map, arena);
    }

    size_t i = (size_t)util_ptr_hash(key);
    for (;;) {
        i &= map->cap - 1;

        if ( !map->keys[i] ) {
            map->keys[i] = key;
            map->vals[i] = val;
            map->len++;

            break;
        } else if ( map->keys[i] == key ) {
            map->vals[i] = val;
        }

        i++;
    }
}

void *map_get(Map *map, void *key) {
    if (map->len == 0) {
        return NULL;
    }

    size_t i = (size_t)util_ptr_hash(key);

    for (;;) {
        i &= map->cap - 1;

        if ( map->keys[i] == key ) {
            return map->vals[i];
        } else if ( !map->keys[i] ) {
            return NULL;
        }
        i++;
    }

    return NULL;
}

struct Buf_Hdr {
    size_t len;
    size_t cap;
    char buf[1];
};

static void *
buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);

    size_t new_cap = CLAMP_MIN(2*buf_cap(buf), MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(Buf_Hdr, buf))/elem_size);
    size_t new_size = offsetof(Buf_Hdr, buf) + new_cap*elem_size;

    Buf_Hdr *new_hdr;
    if (buf) {
        new_hdr = (Buf_Hdr *)realloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = (Buf_Hdr *)malloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

static char *
buf__printf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t cap = buf_cap(buf) - buf_len(buf);
    size_t n = 1 + vsnprintf(buf_end(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        buf_fit(buf, n + buf_len(buf));
        va_start(args, fmt);
        size_t new_cap = buf_cap(buf) - buf_len(buf);
        n = 1 + vsnprintf(buf_end(buf), new_cap, fmt, args);
        assert(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}
