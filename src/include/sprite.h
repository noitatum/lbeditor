#pragma once

#include <SDL2/SDL.h>
#include <stdio.h>

#define SPRITE_BALL_COUNT       0x08
#define SPRITE_LETTER_COUNT     0x5D
#define SPRITE_LIGHT_COUNT      0x03
#define SPRITE_BLOCK_COUNT      0x30
#define SPRITE_SLOPE_COUNT      0x14
#define SPRITE_HOLE_COUNT       0x14
#define SPRITE_MAP_SUM_COUNT    0x58
#define SPRITE_DUST_COUNT       0x04
#define LETTER(letter) ((letter) - 32)

typedef struct lb_sprites {
    SDL_Texture* balls[SPRITE_BALL_COUNT];
    SDL_Texture* letters[SPRITE_LETTER_COUNT];
    SDL_Texture* lights[SPRITE_LIGHT_COUNT];
    SDL_Texture* blocks[SPRITE_BLOCK_COUNT];
    SDL_Texture* slopes[SPRITE_SLOPE_COUNT];
    SDL_Texture* holes[SPRITE_HOLE_COUNT];
    SDL_Texture* crater;
    SDL_Texture* dusts[SPRITE_DUST_COUNT];
} lb_sprites;

SDL_Texture* create_texture(SDL_Renderer* renderer, size_t x, size_t y);
lb_sprites* sprites_init(SDL_Renderer* renderer, FILE* rom);
void sprites_destroy(lb_sprites* sprites);
