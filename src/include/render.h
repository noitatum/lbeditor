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
