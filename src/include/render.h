#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>
#include <integer.h>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 480
#define TSIZE 16
#define BSIZE (TSIZE * 2)

typedef struct rgba_color {
    u8 r, g, b, a;
} rgba_color;

SDL_Renderer* initialize_render(SDL_Window* window);
void render_stage(SDL_Renderer* renderer, lb_sprites* sprites, 
                  table_full* table, stage_ball* balls, table_tiles* tiles);
void render_tiles(SDL_Renderer* renderer, table_tiles* tiles, 
                  lb_sprites* sprites);
void set_render_color(SDL_Renderer* renderer, rgba_color color);
rgba_color get_color(u8 color); 
void printf_pos(SDL_Renderer* renderer, lb_sprites* sprites, 
                size_t x, size_t y, const char* format, ...);
