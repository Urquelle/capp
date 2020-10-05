size_t
string_size( char *str ) {
    size_t result = utf8_str_size(str);

    return result;
}

size_t
string_len( char *str ) {
    size_t result = utf8_str_len(str);

    return result;
}

void
string_concat( char *first, char *second, char *dest ) {
    size_t first_size = string_size(first);
    size_t second_size = string_size(second);

    uint32_t dest_pos = 0;

    for ( size_t i = 0; i < first_size; ++dest_pos, ++i ) {
        dest[dest_pos] = first[i];
    }

    for ( size_t i = 0; i < second_size; ++dest_pos, ++i ) {
        dest[dest_pos] = second[i];
    }
}

void
string_copy( char *source, char *dest, uint64_t len ) {
    size_t source_len = string_len( source );

    for ( uint32_t i = 0; i < len; ++i ) {
        dest[i] = source[i];
    }

    dest[len] = '\0';
}

int
string_to_int(char *str) {
    int result = 0;

    while ( *str ) {
        result *= 10;
        result += *str - '0';
        str++;
    }

    return result;
}

inline bool
is_equal(char *a, char *b, size_t len) {
    bool result = strncmp(a, b, len) == 0;

    return result;
}

static char *
str_intern_range(char *start, char *end, Map *map, Mem_Arena *arena) {
    size_t len = end - start;
    uint64_t hash = util_bytes_hash(start, len);
    void *key = (void *)(uintptr_t)(hash ? hash : 1);

    Str_Intern *intern = (Str_Intern *)map_get(map, key);
    for (Str_Intern *it = intern; it; it = it->next) {
        if (it->length == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }

    Str_Intern *new_intern = (Str_Intern *)ALLOC_SIZE(arena, offsetof(Str_Intern, str) + len + 1);

    new_intern->length = len;
    new_intern->next   = intern;
    memcpy(new_intern->str, start, len);
    new_intern->str[len] = 0;
    map_push(map, key, new_intern, arena);

    return new_intern->str;
}

static char *
str_intern(char *str, Map *map, Mem_Arena *arena) {
    return str_intern_range(str, str + string_len(str), map, arena);
}

char *
strf(Mem_Arena *arena, char *fmt, ...) {
    va_list args = NULL;
    va_start(args, fmt);
    int size = 1 + vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *str = ALLOC_SIZE(arena, size);

    va_start(args, fmt);
    vsnprintf(str, size, fmt, args);
    va_end(args);

    return str;
}

