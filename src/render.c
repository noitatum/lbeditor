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

const size_t hole_order[16] = 
    {15, 2, 3, 7, 1, 6, 12, 10, 0, 13, 4, 11, 5, 9, 8, 14};
const size_t nibble_popcount[16] = 
    {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
// FIXME: Hacky, normalize slopes
const size_t slope_order[16] = {0, 2, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

rgba_color get_color(u8 color) { 
    return NES_PALETTE[color & 0x3F];
}

void set_render_color(SDL_Renderer* renderer, rgba_color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void render_back(SDL_Renderer* renderer, table_full* table) {
    set_render_color(renderer, NES_PALETTE[0x25]);
    SDL_RenderClear(renderer);
    set_render_color(renderer, NES_PALETTE[0x09]);
    for (size_t i = 0; i < table->back_count; i++) {
        table_back* b = table->backs + i;
        SDL_Rect rect = {b->x1 * 16, b->y1 * 16, 
                         (b->x2 - b->x1 + 1) * 16, (b->y2 - b->y1 + 1) * 16};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void render_table(SDL_Renderer* renderer, table_tiles* tiles, 
                  lb_sprites* sprites) { 
    for (size_t i = 1; i < TABLE_MAX_X - 1; i++) {
        for (size_t j = 1; j < TABLE_MAX_Y - 1; j++) {
            u8 tile = tiles->tiles[j][i];
            SDL_Rect dest = {i * 16, j * 16, 16, 16};
            size_t hole = hole_order[tile >> 4];
            if (hole != 15)
                SDL_RenderCopy(renderer, sprites->holes[hole], NULL, &dest);
            size_t block = tile & TILE_MASK_BLOCK;
            if (block) {
                if (nibble_popcount[block] == 1) {
                    SDL_Texture* tte = sprites->slopes[slope_order[block] + 16];
                    SDL_RenderCopy(renderer, tte, NULL, &dest);
                } else {
                    SDL_RenderCopy(renderer, sprites->blocks[2], NULL, &dest);
                }
            }
        }
    }
}

void render_balls(SDL_Renderer* renderer, position* balls, 
                  lb_sprites* sprites) {
    for (size_t i = 0; i < BALL_COUNT; i++) {
        if (balls[i].x == 0)
            continue;
        SDL_Rect dest = {balls[i].y * 2 - 16, balls[i].x * 2 - 16, 32, 32};
        SDL_RenderCopy(renderer, sprites->balls[i], NULL, &dest);
    }
}
