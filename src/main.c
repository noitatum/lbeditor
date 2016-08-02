#include <SDL2/SDL.h>
#include <stdio.h>
#include <integer.h>
#include <sprite.h>
#include <stage.h>
#include <render.h>
#include <hud.h>
#include <action.h>

static const SDL_Rect map_area = {3 * TSIZE, 8 * TSIZE, 26 * TSIZE, 19 * TSIZE};

typedef struct resources {
    FILE* rom;
    SDL_Window* window;
    SDL_Renderer* renderer;
    lb_sprites* sprites;
    lb_stages* stages;
    lb_hud* hud;
    action_history* history;
} resources;

SDL_Window* initialize_sdl() {
    // Try to initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO))
        return NULL;
    // Try to initialize the window
    return SDL_CreateWindow("Moon Editor",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

void resources_destroy(resources* res) {
    if (res->history)
        free(res->history);
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

int resources_init(resources* r, FILE* rom) {
    *r = (resources) {0};
    r->rom = rom;
    if (!(r->window = initialize_sdl()))
        return -1;
    if (!(r->renderer = initialize_render(r->window)))
        return -1;
    if (!(r->sprites = sprites_init(r->renderer, rom)))
        return -1;
    if (!(r->stages = stages_init(rom)))
        return -1;
    if (!(r->hud = hud_init(r->renderer, r->sprites)))
        return -1;
    if (!(r->history = action_history_init()))
        return -1;
    return 0;
}

void exit_error(resources* res) {
    resources_destroy(res);
    fprintf(stderr, "SDL Failure: %s", SDL_GetError());
    exit(EXIT_FAILURE);
}

void handle_event(SDL_Event* e, resources* r, table_tiles* tiles) {
    table_full* table = r->stages->tables + r->hud->map;
    if (e->type == SDL_KEYDOWN) {
        hud_key(r->hud, e->key.keysym.sym);
        if (table != r->stages->tables + r->hud->map) {
            init_table_tiles(tiles, r->stages->tables + r->hud->map);
            action_history_clear(r->history);
        }
        if (e->key.keysym.sym == SDLK_u) {
            action_history_undo(r->history, table, tiles);
            r->history->active = 0;
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        size_t x = e->button.x / TSIZE, y = e->button.y / TSIZE;
        if (e->button.button == SDL_BUTTON_LEFT) {
            if (!in_rect(map_area, e->button.x, e->button.y)) {
                hud_click(r->hud, e->button.x, e->button.y);
                return;
            }
            action_history_do(r->history, table, tiles, hud_tool(r->hud), x, y);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            action_history_undo(r->history, table, tiles);
            r->history->active = 0;
        }
    } else if (e->type == SDL_MOUSEMOTION) {
        size_t x = e->motion.x / TSIZE, y = e->motion.y / TSIZE;
        if (r->history->active && in_rect(map_area, e->button.x, e->button.y))
            action_history_redo(r->history, table, tiles, x, y);
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        if (e->button.button == SDL_BUTTON_LEFT)
            r->history->active = 0;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
       fprintf(stderr, "Usage moon <Lunar Ball Rom>\n");
       return EXIT_FAILURE;
    }
    FILE* rom;
    resources r;
    if (!(rom = fopen(argv[1], "r"))) {
        fprintf(stderr, "Couldn't open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (resources_init(&r, rom))
        exit_error(&r);
    table_tiles tiles;
    SDL_Event e = {0};
    init_table_tiles(&tiles, r.stages->tables);
    SDL_SetRenderTarget(r.renderer, NULL);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        handle_event(&e, &r, &tiles);
        render_all(r.renderer, r.sprites, r.stages, r.hud, &tiles);
        SDL_SetRenderDrawColor(r.renderer, 0x00, 0xFF, 0x00, 0xFF);
        SDL_RenderDrawRect(r.renderer, &map_area);
        SDL_RenderPresent(r.renderer);
    }
    resources_destroy(&r);
    return EXIT_SUCCESS;
}
