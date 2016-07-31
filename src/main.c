#include <SDL2/SDL.h>
#include <stdio.h>
#include <integer.h>
#include <sprite.h>
#include <stage.h>
#include <render.h>
#include <hud.h>

static const SDL_Rect map_area = {3 * TSIZE, 8 * TSIZE, 26 * TSIZE, 19 * TSIZE};

typedef struct resources {
    SDL_Window* window;
    SDL_Renderer* renderer;
    lb_sprites* sprites;
    lb_stages* stages;
    lb_hud* hud;
    FILE* rom;
} resources;

typedef struct tool_action {
    size_t active, tool, x1, y1, x2, y2;
} action_tool;

void resources_cleanup(resources* res) {
    if (res->hud)
        hud_destroy(res->hud);
    if (res->stages)
        free(res->stages);
    if (res->sprites)
        sprites_destroy(res->sprites);
    if (res->renderer)
        SDL_DestroyRenderer(res->renderer);
    if (res->window)
        SDL_DestroyWindow(res->window);
    fclose(res->rom);
    SDL_Quit();
}

void exit_error(resources* res) {
    resources_cleanup(res);
    fprintf(stderr, "SDL Failure: %s", SDL_GetError());
    exit(EXIT_FAILURE);
}

SDL_Window* initialize_sdl() {
    // Try to initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO))
        return NULL;
    // Try to initialize the window
    return SDL_CreateWindow("Moon Editor", 
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                            SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

void handle_action(table_full* table, table_tiles* tiles, action_tool* action) {
    if (action->tool == TOOL_HOLE)
        action->active = !table_add_hole(table, tiles, action->x2, action->y2);
    else if (action->tool == TOOL_BACK)
        action->active = !table_add_back(table, tiles, action->x1, action->y1,
                                         action->x2, action->y2);
    else if (action->tool <= TOOL_BACK)
        action->active = !table_add_line(table, tiles, action->x1, action->y1,
                                         action->x2, action->y2, action->tool);
}

void handle_event(SDL_Event* e, resources* r, table_tiles* tiles,
                  table_tiles* backup, action_tool* action) {
    table_full* table = r->stages->tables + r->hud->map;
    if (e->type == SDL_KEYDOWN) {
        hud_key(r->hud, e->key.keysym.sym);
        if (table != r->stages->tables + r->hud->map) {
            init_table_tiles(tiles, r->stages->tables + r->hud->map);
            *backup = *tiles;
            action->active = 0;
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        size_t x = e->button.x / TSIZE, y = e->button.y / TSIZE;
        if (e->button.button == SDL_BUTTON_LEFT) {
            if (!in_rect(map_area, e->button.x, e->button.y)) {
                hud_click(r->hud, e->button.x, e->button.y);
                return;
            }
            *action = (action_tool) {1, hud_tool(r->hud), x, y, x, y};
            handle_action(table, tiles, action);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            *tiles = *backup;
            action->active = 0;
        }
    } else if (e->type == SDL_MOUSEMOTION) {
        size_t x = e->motion.x / TSIZE, y = e->motion.y / TSIZE;
        if ((x != action->x2 || y != action->y2) &&
            action->active && in_rect(map_area, e->button.x, e->button.y)) {
            action->x2 = x, action->y2 = y;
            *tiles = *backup;
            handle_action(table, tiles, action);
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        if (action->active && e->button.button == SDL_BUTTON_LEFT) {
            if (action->tool == TOOL_HOLE)
                table_increment_holes(r->stages, table);
            else if (action->tool == TOOL_BACK)
                table_increment_backs(r->stages, table);
            else if (action->tool <= TOOL_BACK)
                table_increment_lines(r->stages, table);
            *backup = *tiles;
            action->active = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
       fprintf(stderr, "Usage moon-editor <Lunar Ball Rom>\n");
       return EXIT_FAILURE;
    }
    resources r = {0};
    if (!(r.rom = fopen(argv[1], "r"))) {
        fprintf(stderr, "Couldn't open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (!(r.window = initialize_sdl()))
        exit_error(&r);
    if (!(r.renderer = initialize_render(r.window)))
        exit_error(&r);
    if (!(r.sprites = sprites_init(r.renderer, r.rom)))
        exit_error(&r);
    if (!(r.stages = stages_init(r.rom)))
        exit_error(&r);
    if (!(r.hud = hud_init(r.renderer, r.sprites)))
        exit_error(&r);
    table_tiles tiles, backup;
    SDL_Event e = {0};
    action_tool action = {0};
    init_table_tiles(&tiles, r.stages->tables);
    backup = tiles;
    SDL_SetRenderTarget(r.renderer, NULL);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        handle_event(&e, &r, &tiles, &backup, &action);
        render_all(r.renderer, r.sprites, r.stages, r.hud, &tiles);
        SDL_SetRenderDrawColor(r.renderer, 0x00, 0xFF, 0x00, 0xFF);
        SDL_RenderDrawRect(r.renderer, &map_area);
        SDL_RenderPresent(r.renderer);
    }
    resources_cleanup(&r);
    return EXIT_SUCCESS;
}
