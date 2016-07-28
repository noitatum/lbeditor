#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>

typedef struct lb_hud {
    SDL_Texture* frame;
    SDL_Texture* tools;
    SDL_Texture* balls;
    size_t toolbox, tool; 
} lb_hud;

void render_hud(SDL_Renderer* renderer, lb_hud* hud, 
                lb_sprites* sprites, lb_stages* stages, size_t index);
lb_hud* hud_init(SDL_Renderer* renderer, lb_sprites* sprites);
void hud_destroy(lb_hud* hud);
void hud_click(lb_hud* hud, size_t x, size_t y);
