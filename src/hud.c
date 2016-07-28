#include <hud.h>
#include <render.h>

#define OPTION_TABLE 0
#define OPTION_BALLS 1

static const table_line hud_lines[] = {
    {0x02 | TYPE_HORIZONTAL, 0x01, 0x1D}, {0x02 | TYPE_HORIZONTAL, 0x06, 0x1D},
    {0x01 | TYPE_VERTICAL  , 0x02, 0x05}, {0x0C | TYPE_VERTICAL  , 0x02, 0x05},
    {0x13 | TYPE_VERTICAL  , 0x02, 0x05}, {0x1E | TYPE_VERTICAL  , 0x02, 0x05},
    {0x01 | TYPE_DIAGONAL  , 0x01 | SLOPE_SE, 0x01},
    {0x1E | TYPE_DIAGONAL  , 0x01 | SLOPE_SW, 0x01},
    {0x01 | TYPE_DIAGONAL  , 0x06 | SLOPE_NE, 0x06},
    {0x1E | TYPE_DIAGONAL  , 0x06 | SLOPE_NW, 0x06},
};

static const SDL_Rect hud_back =
    {0x2 * TSIZE, 0x2 * TSIZE, 0x1C * TSIZE, 0x4 * TSIZE};

static const SDL_Rect selection =
    {2 * TSIZE, 2 * TSIZE, 10 * TSIZE, 4 * TSIZE};

SDL_Texture* create_texture_frame(SDL_Renderer* renderer, lb_sprites* sprites) {
    SDL_Texture* hud = create_texture(renderer, SCREEN_WIDTH, SCREEN_HEIGHT); 
    set_render_color(renderer, get_color(0x00));
    SDL_RenderFillRect(renderer, &hud_back);
    table_tiles tiles;
    memset(&tiles, 0, sizeof(tiles));
    tile_table_lines(&tiles, hud_lines, sizeof(hud_lines) / sizeof(table_line));
    render_tiles(renderer, &tiles, sprites);
    return hud;
}

SDL_Texture* create_texture_balls(SDL_Renderer* renderer, lb_sprites* sprites) {
    SDL_Texture* balls = create_texture(renderer, selection.w, selection.h);
    for (size_t i = 0; i < BALL_COUNT; i++) {
        SDL_Rect target = {(i & 0x3) * TSIZE * 2, (i >> 2) * TSIZE * 2,
                           TSIZE * 2, TSIZE * 2}; 
        SDL_RenderCopy(renderer, sprites->balls[i], NULL, &target);
    }
    return balls;
}

void render_texture_tool(SDL_Renderer* renderer, SDL_Texture** corners,
                         size_t x, size_t y) {
    SDL_Rect target = {x + TSIZE, y + TSIZE, TSIZE, TSIZE};
    SDL_RenderCopy(renderer, corners[0], NULL, &target);
    target.x -= TSIZE;
    SDL_RenderCopy(renderer, corners[1], NULL, &target);
    target.y -= TSIZE;
    SDL_RenderCopy(renderer, corners[2], NULL, &target);
    target.x += TSIZE;
    SDL_RenderCopy(renderer, corners[3], NULL, &target);
}

SDL_Texture* create_texture_tools(SDL_Renderer* renderer, lb_sprites* sprites) {
    SDL_Texture* tools = create_texture(renderer, selection.w, selection.h);    
    SDL_Rect target = {0, 0, TSIZE * 2, TSIZE * 2};
    for (size_t i = 0; i < 4; i++) {
        SDL_RenderCopy(renderer, sprites->slopes[16 + i], NULL, &target);
        target.x += TSIZE * 2;
    }
    SDL_RenderCopy(renderer, sprites->blocks[2], NULL, &target);
    target = (SDL_Rect) {0, TSIZE * 2, TSIZE * 2, TSIZE * 2};
    render_texture_tool(renderer, sprites->slopes, target.x, target.y);
    target.x += TSIZE * 2;
    render_texture_tool(renderer, sprites->blocks + 0x20, target.x, target.y);
    target.x += TSIZE * 2;
    render_texture_tool(renderer, sprites->holes, target.x, target.y);
    target.x += TSIZE * 2;
    set_render_color(renderer, get_color(0x09));
    SDL_RenderFillRect(renderer, &target);
    target.x += TSIZE * 2;
    SDL_RenderCopy(renderer, sprites->letters['X'], NULL, &target);
    return tools;
}

void render_hud(SDL_Renderer* renderer, lb_hud* hud, 
                lb_sprites* sprites, lb_stages* stages, size_t index) {
    table_full* table = stages->tables + index;
    SDL_RenderCopy(renderer, hud->frame, NULL, NULL); 
    if (hud->option == OPTION_TABLE)
        SDL_RenderCopy(renderer, hud->tools, NULL, &selection);
    else
        SDL_RenderCopy(renderer, hud->balls, NULL, &selection);
    printf_pos(renderer, sprites, 0xD, 0x2, 
        "map %02i byte %i\nstages line %i\n%02i %02i  hole %i\n       back %i\n"
        , index, table->byte_count, table->line_count, table->stage_a + 1, 
        table->stage_b + 1, table->hole_count, table->back_count);
}

lb_hud* hud_init(SDL_Renderer* renderer, lb_sprites* sprites) {
    lb_hud* hud = malloc(sizeof(lb_hud));
    hud->frame = create_texture_frame(renderer, sprites);
    hud->tools = create_texture_tools(renderer, sprites);
    hud->balls = create_texture_balls(renderer, sprites);
    hud->option = OPTION_TABLE;
    return hud;
}

void hud_destroy(lb_hud* hud) {
    SDL_DestroyTexture(hud->frame);
    SDL_DestroyTexture(hud->tools);
    SDL_DestroyTexture(hud->balls);
    free(hud);
}
