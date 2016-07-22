#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>
#include <integer.h>

typedef struct rgba_color {
    u8 r, g, b, a;
} rgba_color;

void render_table(SDL_Renderer* renderer, table_tiles* tiles, 
                  lb_sprites* sprites); 
void render_balls(SDL_Renderer* renderer, position* balls, lb_sprites* sprites);
void set_render_color(SDL_Renderer* renderer, rgba_color color);
rgba_color get_color(u8 color); 
