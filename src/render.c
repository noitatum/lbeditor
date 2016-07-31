#include <SDL2/SDL.h>
#include <render.h>

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

const size_t hole_order[16] = {0xF, 0x2, 0x3, 0x7, 0x1, 0x6, 0xC, 0xA, 
                               0x0, 0xD, 0x4, 0xB, 0x5, 0x9, 0x8, 0xE};
const size_t slope_order[4] = {0, 1, 3, 2};
const size_t slope_table[8] = {4, 3, 4, 3, 2, 1, 2, 0};
const size_t block_order[256] = {
    // 0x00
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    // 0x10
    0x2A, 0x2D, 0x2A, 0x2D, 0x25, 0x1D, 0x25, 0x19,
    0x2A, 0x2D, 0x2A, 0x2D, 0x21, 0x15, 0x21, 0x11,
    // 0x20 (Same as 0x00)
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    // 0x30 (Same as 0x10)
    0x2A, 0x2D, 0x2A, 0x2D, 0x25, 0x1D, 0x25, 0x19,
    0x2A, 0x2D, 0x2A, 0x2D, 0x21, 0x15, 0x21, 0x11,
    // 0x40
    0x2B, 0x27, 0x2B, 0x27, 0x2C, 0x1C, 0x2C, 0x14,
    0x2B, 0x27, 0x2B, 0x27, 0x2C, 0x1C, 0x2C, 0x14,
    // 0x50
    0x26, 0x1F, 0x26, 0x1F, 0x1E, 0x03, 0x1E, 0x0C,
    0x26, 0x1F, 0x26, 0x1F, 0x1A, 0x0D, 0x1A, 0x09,
    // 0x60 (Same as 0x40)
    0x2B, 0x27, 0x2B, 0x27, 0x2C, 0x1C, 0x2C, 0x14,
    0x2B, 0x27, 0x2B, 0x27, 0x2C, 0x1C, 0x2C, 0x14,
    // 0x70
    0x22, 0x1B, 0x22, 0x1B, 0x16, 0x0E, 0x16, 0x2F,
    0x22, 0x1B, 0x22, 0x1B, 0x12, 0x0A, 0x12, 0x05,
    // 0x80 (Same as 0x00)
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    // 0x90 (Same as 0x10)
    0x2A, 0x2D, 0x2A, 0x2D, 0x25, 0x1D, 0x25, 0x19,
    0x2A, 0x2D, 0x2A, 0x2D, 0x21, 0x15, 0x21, 0x11,
    // 0xA0 (Same as 0x00)
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    0x02, 0x28, 0x02, 0x28, 0x29, 0x24, 0x29, 0x20,
    // 0xB0 (Same as 0x10)
    0x2A, 0x2D, 0x2A, 0x2D, 0x25, 0x1D, 0x25, 0x19,
    0x2A, 0x2D, 0x2A, 0x2D, 0x21, 0x15, 0x21, 0x11,
    // 0xC0
    0x2B, 0x23, 0x2B, 0x23, 0x2C, 0x18, 0x2C, 0x10,
    0x2B, 0x23, 0x2B, 0x23, 0x2C, 0x18, 0x2C, 0x10,
    // 0xD0
    0x26, 0x17, 0x26, 0x17, 0x1E, 0x0F, 0x1E, 0x08,
    0x26, 0x17, 0x26, 0x17, 0x1A, 0x2E, 0x1A, 0x04,
    // 0xE0 (Same as 0xC0)
    0x2B, 0x23, 0x2B, 0x23, 0x2C, 0x18, 0x2C, 0x10,
    0x2B, 0x23, 0x2B, 0x23, 0x2C, 0x18, 0x2C, 0x10,
    // 0xF0
    0x22, 0x13, 0x22, 0x13, 0x16, 0x0B, 0x16, 0x07,
    0x22, 0x13, 0x22, 0x13, 0x12, 0x06, 0x12, 0x01,
};

const int sur_offset_x[9] = {-1, -1, 0, 1, 1,  1,  0, -1, -1};
const int sur_offset_y[9] = { 0,  1, 1, 1, 0, -1, -1, -1,  0};
const u8 sur_flags[9] = {0x4, 0x6, 0x2, 0x3, 0x1, 0x9, 0x8, 0xC, 0x4};

rgba_color get_color(u8 color) { 
    return NES_PALETTE[color & 0x3F];
}

void set_render_color(SDL_Renderer* renderer, rgba_color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

int random_tile(SDL_Rect* rect, size_t big, 
                size_t size_x, size_t size_y, u8 set[size_y][size_x]) {
    int x = rand() % (size_x - 1), y = rand() % (size_y - 1);
    for (size_t j = y; j <= y + big; j++)
        for (size_t i = x; i <= x + big; i++)
            if (set[j][i])
                return -1;
    for (size_t j = y; j <= y + big; j++)
        for (size_t i = x; i <= x + big; i++)
            set[j][i] = 1;
    rect->x = (x - 1) * TSIZE;
    rect->y = (y - 1) * TSIZE;
    return 0;
}

void render_dust(SDL_Renderer* renderer, lb_sprites* sprites) {
    size_t size_x = GRID_WIDTH + 2, size_y = GRID_HEIGHT + 2;
    u8 dust[size_y][size_x];
    memset(dust, 0, sizeof(dust));
    set_render_color(renderer, NES_PALETTE[0x25]);
    SDL_RenderClear(renderer);
    SDL_Rect dest = (SDL_Rect) {0, 0, TSIZE * 2, TSIZE * 2};
    for (size_t i = 0; i < 10; i++)
        if (!random_tile(&dest, 1, size_x, size_y, dust))
            SDL_RenderCopy(renderer, sprites->crater, NULL, &dest);
    dest = (SDL_Rect) {0, 0, TSIZE, TSIZE};
    for (size_t i = 0; i < 100; i++)
        if (!random_tile(&dest, 0, size_x, size_y, dust))
            SDL_RenderCopy(renderer, sprites->dusts[rand() % 2], NULL, &dest);
    for (size_t i = 0; i < 500; i++)
        if (!random_tile(&dest, 0, size_x, size_y, dust))
            SDL_RenderCopy(renderer, sprites->dusts[3], NULL, &dest);
}

void render_back(SDL_Renderer* renderer, table_tiles* tiles) {
    set_render_color(renderer, NES_PALETTE[0x09]);
    for (size_t i = 0; i < tiles->back_count; i++) {
        table_back* b = tiles->backs + i;
        SDL_Rect rect = {b->x1 * TSIZE, b->y1 * TSIZE, 
                         (b->x2 - b->x1 + 1) * TSIZE, 
                         (b->y2 - b->y1 + 1) * TSIZE};
        SDL_RenderFillRect(renderer, &rect);
    }
}

u16 get_surroundings(table_tiles* tiles, size_t y, size_t x) {
    u16 sur = 0; 
    for (size_t i = 0; i < 9; i++) {
        u8 wall = tiles->walls[y + sur_offset_y[i]][x + sur_offset_x[i]];
        sur = (sur << 1) | ((wall & sur_flags[i]) == sur_flags[i]);
    }
    return sur;
}

void render_holes(SDL_Renderer* renderer, table_tiles* tiles,
                  lb_sprites* sprites) { 
    for (size_t j = 1; j < GRID_HEIGHT - 1; j++) {
        for (size_t i = 1; i < GRID_WIDTH - 1; i++) {
            SDL_Rect dest = {i * TSIZE, j * TSIZE, TSIZE, TSIZE};
            size_t hole = hole_order[tiles->holes[j][i]];
            if (hole != 0xF)
                SDL_RenderCopy(renderer, sprites->holes[hole], NULL, &dest);
        }
    }
}

void render_walls(SDL_Renderer* renderer, table_tiles* tiles,
                  lb_sprites* sprites) { 
    for (size_t j = 1; j < GRID_HEIGHT - 1; j++) {
        for (size_t i = 1; i < GRID_WIDTH - 1; i++) {
            SDL_Rect dest = {i * TSIZE, j * TSIZE, TSIZE, TSIZE};
            size_t wall = tiles->walls[j][i];
            if (!wall)
                continue;
            // FIXME: This is not how it works in the game
            u16 sur = get_surroundings(tiles, j, i);
            if (wall == TILE_MASK_BLOCK) {
                SDL_Texture* block = sprites->blocks[block_order[sur & 0xFF]];
                SDL_RenderCopy(renderer, block, NULL, &dest);
            } else {
                size_t slope = slope_order[wall / 3 - 1];
                size_t index = slope_table[(sur >> (slope * 2)) & 0x7];
                SDL_Texture* tte = sprites->slopes[index * 4 + slope];
                SDL_RenderCopy(renderer, tte, NULL, &dest);
            }
        }
    }
}

void render_balls(SDL_Renderer* renderer, stage_ball* balls, 
                  lb_sprites* sprites) {
    for (size_t i = 0; i < BALL_COUNT; i++) {
        if (balls[i].x == 0)
            continue;
        SDL_Rect dest = {balls[i].y * 2 - 16, balls[i].x * 2 - 16, 32, 32};
        SDL_RenderCopy(renderer, sprites->balls[i], NULL, &dest);
    }
}

void render_all(SDL_Renderer* renderer, lb_sprites* sprites, lb_stages* stages,
                lb_hud* hud, table_tiles* tiles) {
    stage_ball* balls = stages->balls[hud->map + TABLE_COUNT * hud->stage_b];
    srand(hud->map);
    render_dust(renderer, sprites);
    render_back(renderer, tiles);
    render_holes(renderer, tiles, sprites);
    render_walls(renderer, tiles, sprites); 
    render_balls(renderer, balls, sprites);
    render_hud(renderer, hud, sprites, stages);
}

SDL_Renderer* initialize_render(SDL_Window* window) {
    SDL_Renderer* re = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!re)
        return NULL;
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    // FIXME: Why is this needed? 
    // First sprite won't load properly without drawing a non transparent point
    SDL_SetRenderDrawColor(re, 255, 255, 255, 255);
    SDL_RenderDrawPoint(re, 0, 0);
    return re;
}

void printf_pos(SDL_Renderer* renderer, lb_sprites* sprites, 
                size_t x, size_t y, const char* format, ...) {
    va_list l;
    char buffer[GRID_WIDTH * GRID_HEIGHT] = {0};
    va_start(l, format);
    vsnprintf(buffer, GRID_WIDTH * GRID_HEIGHT, format, l);
    va_end(l);
    SDL_Rect target = {x * TSIZE, y * TSIZE, TSIZE, TSIZE};
    for (size_t i = 0; i < GRID_WIDTH * GRID_HEIGHT && buffer[i]; i++) {
        if (buffer[i] == '\n') {
            target.x = x * TSIZE;
            target.y += TSIZE;
            continue;
        }
        SDL_RenderCopy(renderer, sprites->letters[buffer[i] - 32], 0, &target);
        target.x += TSIZE;
    }
}
