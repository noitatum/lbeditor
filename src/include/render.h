#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>
#include <hud.h>
#include <integer.h>

#define TSIZE         16
#define BSIZE         (TSIZE * 2)
#define SCREEN_WIDTH  (TSIZE * GRID_WIDTH)
#define SCREEN_HEIGHT (TSIZE * GRID_HEIGHT)

typedef struct rgba_color {
    u8 r, g, b, a;
} rgba_color;

SDL_Renderer* initialize_render(SDL_Window* window);
void render_walls(SDL_Renderer* renderer, table_tiles* tiles, 
                  lb_sprites* sprites);
void render_all(SDL_Renderer* renderer, lb_sprites* sprites, lb_stages* stages,
                lb_hud* hud, table_tiles* tiles);
void set_render_color(SDL_Renderer* renderer, rgba_color color);
rgba_color get_color(u8 color); 
void printf_pos(SDL_Renderer* renderer, lb_sprites* sprites, 
                size_t x, size_t y, const char* format, ...);
