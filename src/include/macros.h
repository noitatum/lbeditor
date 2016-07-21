#ifndef __MACROS_H__
#define __MACROS_H__

#define SDL_RET_IF_ERROR(EXPR) \
    if ((EXPR) < 0) { \
        fprintf(stderr, "SDL Failure: %s", SDL_GetError()); \
        return -1; \
    }

#define SDL_RET_IF_NULL(EXPR) \
    if (!(EXPR)) { \
        fprintf(stderr, "SDL Failure: %s", SDL_GetError()); \
        return -1; \
    }

#define RET_IF_TRUE(EXPR, ...) \
    if (EXPR) { \
        fprintf(stderr, __VA_ARGS__); \
        return -1; \
    }

#define RET_IF_FALSE(EXPR, ...) \
    if (!(EXPR)) { \
        fprintf(stderr, __VA_ARGS__); \
        return -1; \
    }

#endif
