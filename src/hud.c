#include <hud.h>
#include <render.h>

static const table_line hud_lines[] = {
    {0x02 | TYPE_HORIZONTAL, 0x01, 0x1D}, {0x02 | TYPE_HORIZONTAL, 0x06, 0x1D},
    {0x01 | TYPE_VERTICAL  , 0x02, 0x05}, {0x0C | TYPE_VERTICAL  , 0x02, 0x05},
    {0x13 | TYPE_VERTICAL  , 0x02, 0x05}, {0x1E | TYPE_VERTICAL  , 0x02, 0x05},
    {0x01 | TYPE_DIAGONAL  , 0x01 | SLOPE_SE, 0x01},
    {0x1E | TYPE_DIAGONAL  , 0x01 | SLOPE_SW, 0x01},
    {0x01 | TYPE_DIAGONAL  , 0x06 | SLOPE_NE, 0x06},
    {0x1E | TYPE_DIAGONAL  , 0x06 | SLOPE_NW, 0x06},
};

const SDL_Rect hud_back = {2 * TSIZE, 2 * TSIZE, 28 * TSIZE, 4 * TSIZE};
const SDL_Rect toolbox =  {2 * TSIZE, 2 * TSIZE, 10 * TSIZE, 4 * TSIZE};

size_t in_rect(SDL_Rect r, ssize_t x, ssize_t y) {
    return x >= r.x && y >= r.y && x - r.x < r.w && y - r.y < r.h;
}

SDL_Texture* create_texture_frame(SDL_Renderer* renderer, lb_sprites* sprites) {
    SDL_Texture* hud = create_texture(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    set_render_color(renderer, get_color(COLOR_GRAY));
    SDL_RenderFillRect(renderer, &hud_back);
    table_tiles tiles;
    memset(&tiles, 0, sizeof(tiles));
    tile_table_lines(&tiles, hud_lines, sizeof(hud_lines) / sizeof(table_line));
    render_walls(renderer, &tiles, sprites);
    return hud;
}

void render_texture_next(SDL_Renderer* renderer, lb_sprites* sprites) {
    static const SDL_Rect target = {TSIZE * 8 + 6, TSIZE * 2 + 2, BSIZE, BSIZE};
    SDL_RenderCopy(renderer, sprites->letters[LETTER('+')], NULL, &target);
}

SDL_Texture* create_texture_balls(SDL_Renderer* renderer, lb_sprites* sprites) {
    SDL_Texture* balls = create_texture(renderer, toolbox.w, toolbox.h);
    for (size_t i = 0; i < BALL_COUNT; i++) {
        SDL_Rect target = {(i % 5) * BSIZE, (i / 5) * BSIZE, BSIZE, BSIZE};
        SDL_RenderCopy(renderer, sprites->balls[i], NULL, &target);
    }
    render_texture_next(renderer, sprites);
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
    SDL_Texture* tools = create_texture(renderer, toolbox.w, toolbox.h);
    SDL_Rect target = {0, 0, BSIZE, BSIZE};
    for (size_t i = 0; i < 4; i++) {
        SDL_RenderCopy(renderer, sprites->slopes[16 + i], NULL, &target);
        target.x += TSIZE * 2;
    }
    SDL_RenderCopy(renderer, sprites->blocks[2], NULL, &target);
    target = (SDL_Rect) {0, BSIZE, BSIZE, BSIZE};
    render_texture_tool(renderer, sprites->slopes, target.x, target.y);
    target.x += TSIZE * 2;
    render_texture_tool(renderer, sprites->blocks + 0x20, target.x, target.y);
    target.x += TSIZE * 2;
    render_texture_tool(renderer, sprites->holes, target.x, target.y);
    target.x += TSIZE * 2;
    set_render_color(renderer, get_color(COLOR_BACK));
    SDL_RenderFillRect(renderer, &target);
    render_texture_next(renderer, sprites);
    return tools;
}

void hud_click(lb_hud* hud, size_t x, size_t y, size_t* invalid_layers) {
    if (in_rect(toolbox, x, y)) {
        hud->tool = (x - toolbox.x) / BSIZE + ((y - toolbox.y) / BSIZE) * 5;
        if (hud->tool == TOOL_NEXT)
            hud->toolbox = !hud->toolbox;
        *invalid_layers |= 1 << LAYER_HUD;
    }
}

void hud_key(lb_hud* hud, SDL_Keycode key, size_t* invalid_layers) {
    if (key == SDLK_LEFT || key == SDLK_RIGHT) {
        if (key == SDLK_RIGHT) {
            hud->map++;
            if (hud->map == TABLE_COUNT)
                hud->map = 0;
        } else {
            if (hud->map == 0)
                hud->map = TABLE_COUNT;
            hud->map--;
        }
        *invalid_layers |= LAYER_FLAGS_ALL;
    } else if (key == SDLK_UP) {
        hud->stage_b = !hud->stage_b;
        *invalid_layers |= 1 << LAYER_BALLS | 1 << LAYER_DUST;
    } else if (key == SDLK_DOWN) {
        hud->toolbox = !hud->toolbox;
        *invalid_layers |= 1 << LAYER_HUD;
    } else if (key >= SDLK_0 && key <= SDLK_9) {
        hud->tool = (key + 1) % TOOLBOX_COUNT;
        *invalid_layers |= 1 << LAYER_HUD;
    }
}

size_t hud_tool(lb_hud* hud) {
    return hud->tool + hud->toolbox * TOOLBOX_COUNT;
}

lb_hud* hud_init(SDL_Renderer* renderer, lb_sprites* sprites) {
    lb_hud* hud = malloc(sizeof(lb_hud));
    *hud = (lb_hud) {0};
    hud->frame = create_texture_frame(renderer, sprites);
    hud->tools = create_texture_tools(renderer, sprites);
    hud->balls = create_texture_balls(renderer, sprites);
    return hud;
}

void hud_destroy(lb_hud* hud) {
    SDL_DestroyTexture(hud->frame);
    SDL_DestroyTexture(hud->tools);
    SDL_DestroyTexture(hud->balls);
    free(hud);
}
