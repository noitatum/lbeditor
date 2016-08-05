#include <SDL2/SDL.h>
#include <render.h>

static const rgba_color NES_PALETTE[64] = {
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
const size_t hole_order[16] = {0xF, 0x2, 0x3, 0x7, 0x1, 0x6, 0xC, 0xA,
                               0x0, 0xD, 0x4, 0xB, 0x5, 0x9, 0x8, 0xE};

const SDL_Rect screen_area = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

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

void render_dust(SDL_Renderer* renderer, lb_sprites* sprites, size_t stage) {
    const rgba_color dust_colors[4] = {
        NES_PALETTE[COLOR_PINK], NES_PALETTE[COLOR_GREEN],
        NES_PALETTE[COLOR_CYAN], NES_PALETTE[COLOR_PURPLE]};
    srand(stage);
    size_t size_x = GRID_WIDTH + 2, size_y = GRID_HEIGHT + 2;
    u8 dust[size_y][size_x];
    memset(dust, 0, sizeof(dust));
    set_render_color(renderer, dust_colors[(stage - 1) % 4]);
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

void render_back(SDL_Renderer* renderer, map_full* map) {
    set_render_color(renderer, NES_PALETTE[COLOR_BACK]);
    for (size_t i = 0; i < map->back_count; i++) {
        map_back* b = map->backs + i;
        SDL_Rect rect = {b->x1 * TSIZE, b->y1 * TSIZE,
                         (b->x2 - b->x1 + 1) * TSIZE,
                         (b->y2 - b->y1 + 1) * TSIZE};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void render_holes(SDL_Renderer* renderer, map_tiles* tiles,
                  lb_sprites* sprites) {
    for (size_t j = 1; j < GRID_HEIGHT - 1; j++) {
        for (size_t i = 1; i < GRID_WIDTH - 1; i++) {
            SDL_Rect dest = {i * TSIZE, j * TSIZE, TSIZE, TSIZE};
            size_t hole = hole_order[tiles->tiles[j][i].hole_flags];
            if (hole != 0xF)
                SDL_RenderCopy(renderer, sprites->holes[hole], NULL, &dest);
        }
    }
}

static const u8 type_table[32] = {
    0x0, 0x3, 0x6, 0xF, 0xC, 0xF, 0xF, 0xF, 
    0x9, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
};

u16 get_surroundings(map_tiles* tiles, size_t y, size_t x) {
    static const int offset_x[9] = {-1, -1, 0, 1, 1,  1,  0, -1, -1};
    static const int offset_y[9] = { 0,  1, 1, 1, 0, -1, -1, -1,  0};
    static const u8 flags[9] = {0x4, 0x6, 0x2, 0x3, 0x1, 0x9, 0x8, 0xC, 0x4};
    u16 sur = 0;
    for (size_t i = 0; i < 9; i++) {
        u8 type = tiles->tiles[y + offset_y[i]][x + offset_x[i]].wall_flags;
        sur = (sur << 1) | ((type_table[type] & flags[i]) == flags[i]);
    }
    return sur;
}

void render_walls(SDL_Renderer* renderer, map_tiles* tiles,
                  lb_sprites* sprites) {
    for (size_t j = 1; j < GRID_HEIGHT - 1; j++) {
        for (size_t i = 1; i < GRID_WIDTH - 1; i++) {
            SDL_Rect dest = {i * TSIZE, j * TSIZE, TSIZE, TSIZE};
            size_t type = type_table[tiles->tiles[j][i].wall_flags];
            if (!type)
                continue;
            // FIXME: This is not how it works in the game
            u16 sur = get_surroundings(tiles, j, i);
            if (type == TILE_BLOCK_FLAGS) {
                SDL_Texture* block = sprites->blocks[block_order[sur & 0xFF]];
                SDL_RenderCopy(renderer, block, NULL, &dest);
            } else {
                static const size_t slope_table[8] = {4, 3, 4, 3, 2, 1, 2, 0};
                static const size_t slope_order[4] = {0, 1, 3, 2};
                size_t slope = slope_order[type / 3 - 1];
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
        SDL_Rect dest = {balls[i].x * 2 - 16, balls[i].y * 2 - 16, 32, 32};
        SDL_RenderCopy(renderer, sprites->balls[i], NULL, &dest);
    }
}

void render_hud(SDL_Renderer* renderer, lb_hud* hud,
                lb_sprites* sprites, map_full* map) {
    extern const SDL_Rect toolbox;
    // Render toolbox
    SDL_RenderCopy(renderer, hud->frame, NULL, &screen_area);
    if (hud->toolbox == TOOLBOX_MAP)
        SDL_RenderCopy(renderer, hud->tools, NULL, &toolbox);
    else
        SDL_RenderCopy(renderer, hud->balls, NULL, &toolbox);
    // Render stage lights
    SDL_Rect light_box = {TSIZE * 15, TSIZE * 4, TSIZE, TSIZE};
    SDL_RenderCopy(renderer, sprites->lights[hud->stage_b], NULL, &light_box);
    light_box.x += TSIZE * 3;
    SDL_RenderCopy(renderer, sprites->lights[!hud->stage_b], NULL, &light_box);
    // Render hud information
    printf_pos(renderer, sprites, 13, 2, "map %02i byte %i\nstages line %i\n"
        "%02i %02i  hole %i\n       back %i\n", hud->map, map->byte_count,
        map->line_count, map->stages[0], map->stages[1],
        map->hole_count, map->back_count);
    // Render selected tool outline
    SDL_Rect selected = {BSIZE * (hud->tool % 5) + toolbox.x,
                         BSIZE * (hud->tool / 5) + toolbox.y, BSIZE, BSIZE};
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderDrawRect(renderer, &selected);
}

void render_invalid(lb_render* render, lb_sprites* sprites, map_full* map,
                    lb_hud* hud, map_tiles* tiles) {
    // Clear invalid layers
    for (size_t i = 0; i < LAYER_COUNT; i++) {
        if (render->invalid_layers & (1 << i)) {
            SDL_SetRenderTarget(render->renderer, render->layers[i]);
            SDL_SetRenderDrawColor(render->renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(render->renderer);
        }
    }
    // Render again only the invalid layers
    if (render->invalid_layers & (1 << LAYER_DUST)) {
        SDL_SetRenderTarget(render->renderer, render->layers[LAYER_DUST]);
        render_dust(render->renderer, sprites, map->stages[hud->stage_b]);
    }
    if (render->invalid_layers & (1 << LAYER_BACK)) {
        SDL_SetRenderTarget(render->renderer, render->layers[LAYER_BACK]);
        render_back(render->renderer, map);
    }
    if (render->invalid_layers & (1 << LAYER_HOLES)) {
        SDL_SetRenderTarget(render->renderer, render->layers[LAYER_HOLES]);
        render_holes(render->renderer, tiles, sprites);
    }
    if (render->invalid_layers & (1 << LAYER_WALLS)) {
        SDL_SetRenderTarget(render->renderer, render->layers[LAYER_WALLS]);
        render_walls(render->renderer, tiles, sprites);
    }
    if (render->invalid_layers & (1 << LAYER_BALLS)) {
        SDL_SetRenderTarget(render->renderer, render->layers[LAYER_BALLS]);
        render_balls(render->renderer, map->balls[hud->stage_b], sprites);
    }
    if (render->invalid_layers & (1 << LAYER_HUD)) {
        SDL_SetRenderTarget(render->renderer, render->layers[LAYER_HUD]);
        render_hud(render->renderer, hud, sprites, map);
    }
    render->invalid_layers = 0;
}

void render_present(lb_render* render) {
    extern const SDL_Rect map_area;
    SDL_SetRenderTarget(render->renderer, NULL);
    for (size_t i = 0; i < LAYER_COUNT; i++)
        SDL_RenderCopy(render->renderer, render->layers[i], NULL, &screen_area);
    SDL_SetRenderDrawColor(render->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDrawRect(render->renderer, &map_area);
    SDL_RenderPresent(render->renderer);
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
        SDL_Texture* letter = sprites->letters[LETTER(buffer[i])];
        SDL_RenderCopy(renderer, letter, 0, &target);
        target.x += TSIZE;
    }
}

lb_render* render_init(SDL_Window* window) {
    lb_render* render = malloc(sizeof(lb_render));
    SDL_Renderer* re = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!(render->renderer = re))
        return NULL;
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    // FIXME: Why is this needed?
    // First sprite won't load properly without drawing a non transparent point
    SDL_SetRenderDrawColor(re, 255, 255, 255, 255);
    SDL_RenderDrawPoint(re, 0, 0);
    for (size_t i = 0; i < LAYER_COUNT; i++)
        render->layers[i] = create_texture(re, SCREEN_WIDTH, SCREEN_HEIGHT);
    render->invalid_layers = LAYER_FLAGS_ALL;
    return render;
}

void render_destroy(lb_render* render) {
    SDL_DestroyRenderer(render->renderer);
    for (size_t i = 0; i < LAYER_COUNT; i++)
        SDL_DestroyTexture(render->layers[i]);
}
