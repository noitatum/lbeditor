#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>
#include <hud.h>
#include <integer.h>

#define TSIZE           16
#define BSIZE           (TSIZE * 2)
#define SCREEN_WIDTH    (TSIZE * GRID_WIDTH)
#define SCREEN_HEIGHT   (TSIZE * GRID_HEIGHT)
#define LAYER_DUST      0
#define LAYER_BACK      1
#define LAYER_HOLES     2
#define LAYER_WALLS     3
#define LAYER_BALLS     4
#define LAYER_HUD       5
#define LAYER_COUNT     6
#define LAYER_FLAGS_ALL ((1 << LAYER_COUNT) - 1)
#define COLOR_GRAY      0x00
#define COLOR_BACK      0x09
#define COLOR_PURPLE    0x14
#define COLOR_GREEN     0x1A
#define COLOR_BLACK     0x1D
#define COLOR_PINK      0x25
#define COLOR_WHITE     0x30
#define COLOR_CYAN      0x3B
#define COLOR_NONE      0x3F

typedef struct rgba_color {
    u8 r, g, b, a;
} rgba_color;

typedef struct lb_render {
    SDL_Renderer* renderer;
    size_t invalid_layers;
    SDL_Texture* layers[LAYER_COUNT];
} lb_render;

void render_walls(SDL_Renderer* renderer, table_tiles* tiles,
                  lb_sprites* sprites);
void render_invalid(lb_render* render, lb_sprites* sprites, lb_stages* stages,
                    lb_hud* hud, table_tiles* tiles);
void render_present(lb_render* render);
void set_render_color(SDL_Renderer* renderer, rgba_color color);
rgba_color get_color(u8 color);
void printf_pos(SDL_Renderer* renderer, lb_sprites* sprites,
                size_t x, size_t y, const char* format, ...);
lb_render* render_init(SDL_Window* window);
void render_destroy(lb_render* render);
