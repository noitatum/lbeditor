#pragma once

#include <SDL2/SDL.h>
#include <sprite.h>
#include <stage.h>

#define TOOLBOX_TABLE 0
#define TOOLBOX_COUNT 10
#define TOOL_BLOCK  4
#define TOOL_SLANT  5
#define TOOL_SQUARE 6
#define TOOL_HOLE   7
#define TOOL_BACK   8

typedef struct lb_hud {
    size_t tool, toolbox, map, stage_b; 
    SDL_Texture* frame;
    SDL_Texture* tools;
    SDL_Texture* balls;
} lb_hud;

size_t in_rect(SDL_Rect r, ssize_t x, ssize_t y);
void render_hud(SDL_Renderer* renderer, lb_hud* hud, 
                lb_sprites* sprites, lb_stages* stages);
lb_hud* hud_init(SDL_Renderer* renderer, lb_sprites* sprites);
void hud_destroy(lb_hud* hud);
void hud_click(lb_hud* hud, size_t x, size_t y, size_t* invalid_layers);
void hud_key(lb_hud* hud, SDL_Keycode key, size_t* invalid_layers);
size_t hud_tool(lb_hud* hud);
