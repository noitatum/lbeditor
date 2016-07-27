#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>
#include <integer.h>

#define TSIZE 16

typedef struct rgba_color {
    u8 r, g, b, a;
} rgba_color;

void render_stage(SDL_Renderer* renderer, lb_sprites* sprites, 
                  table_full* table, stage_ball* balls, table_tiles* tiles);
void set_render_color(SDL_Renderer* renderer, rgba_color color);
rgba_color get_color(u8 color); 
SDL_Renderer* initialize_render(SDL_Window* window);
