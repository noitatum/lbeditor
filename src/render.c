#include <SDL2/SDL.h>
#include <render.h>
#include <stage.h>
#include <sprite.h>

const rgba_color NES_PALETTE[64] = {
    { 84, 84, 84,255}, {  0, 30,116,255}, {  8, 16,144,255}, { 48,  0,136,255},
    { 68,  0,100,255}, { 92,  0, 48,255}, { 84,  4,  0,255}, { 60, 24,  0,255},
    { 32, 42,  0,255}, {  8, 58,  0,255}, {  0, 64,  0,255}, {  0, 60,  0,255},
    {  0, 50, 60,255}, {  0,  0,  0,255}, {  0,  0,  0,  0}, {  0,  0,  0,  0},
    {152,150,152,255}, {  8, 76,196,255}, { 48, 50,236,255}, { 92, 30,228,255},
    {136, 20,176,255}, {160, 20,100,255}, {152, 34, 32,255}, {120, 60,  0,255},
    { 84, 90,  0,255}, { 40,114,  0,255}, {  8,124,  0,255}, {  0,118, 40,255},
    {  0,102,120,255}, {  0,  0,  0,255}, {  0,  0,  0,  0}, {  0,  0,  0,  0},
    {236,238,236,255}, { 76,154,236,255}, {120,124,236,255}, {176, 98,236,255},
    {228, 84,236,255}, {236, 88,180,255}, {236,106,100,255}, {212,136, 32,255},
    {160,170,  0,255}, {116,196,  0,255}, { 76,208, 32,255}, { 56,204,108,255},
    { 56,180,204,255}, { 60, 60, 60,255}, {  0,  0,  0,  0}, {  0,  0,  0,  0},
    {236,238,236,255}, {168,204,236,255}, {188,188,236,255}, {212,178,236,255},
    {236,174,236,255}, {236,174,212,255}, {236,180,176,255}, {228,196,144,255},
    {204,210,120,255}, {180,222,120,255}, {168,226,144,255}, {152,226,180,255},
    {160,214,228,255}, {160,162,160,255}, {  0,  0,  0,  0}, {  0,  0,  0,  0},
};

rgba_color get_color(u8 color) { 
    return NES_PALETTE[color & 0x3F];
}

void set_render_color(SDL_Renderer* renderer, rgba_color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void render_table(SDL_Renderer* renderer, table_tiles* tiles, 
                  lb_sprites* sprites) { 
    set_render_color(renderer, NES_PALETTE[0x25]);
    SDL_RenderClear(renderer);
    for (size_t i = 1; i < TABLE_MAX_X - 1; i++) {
        for (size_t j = 1; j < TABLE_MAX_Y - 1; j++) {
            u8 tile = tiles->tiles[j][i];
            if (!tile)
                continue;
            SDL_Rect dest = {i * 16, j * 16, 16, 16};
            if (tile & TILE_FLAG_BACK) 
                SDL_RenderCopy(renderer, sprites->blocks[0], NULL, &dest); 
            if (tile & TILE_FLAG_HOLE)
                SDL_RenderCopy(renderer, sprites->holes[0], NULL, &dest);
            if (tile & TILE_FLAG_BLOCK)
                SDL_RenderCopy(renderer, sprites->blocks[2], NULL, &dest);
        }
    }
}
