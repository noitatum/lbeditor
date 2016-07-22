#pragma once

#include <SDL2/SDL.h>
#include <stdio.h>

#define SPRITE_BALL_COUNT   0x08
#define SPRITE_LETTER_COUNT 0x60
#define SPRITE_BLOCK_COUNT  0x30
#define SPRITE_SLOPE_COUNT  0x14
#define SPRITE_HOLE_COUNT   0x14
#define SPRITE_MOON_COUNT   0x08
#define SPRITE_RIGHT_COUNT  0xC0

typedef struct lb_sprites {
    SDL_Texture* balls[SPRITE_BALL_COUNT];
    SDL_Texture* letters[SPRITE_LETTER_COUNT];
    SDL_Texture* blocks[SPRITE_BLOCK_COUNT];
    SDL_Texture* slopes[SPRITE_SLOPE_COUNT];
    SDL_Texture* holes[SPRITE_HOLE_COUNT];
    SDL_Texture* moons[SPRITE_MOON_COUNT];
} lb_sprites;

lb_sprites* sprites_init(SDL_Renderer* renderer, FILE* rom);
void sprites_destroy(lb_sprites* sprites);
