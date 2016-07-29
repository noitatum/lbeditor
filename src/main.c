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

void handle_event(SDL_Event* e, resources* r, table_tiles* tiles) {
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode key = e->key.keysym.sym;
        if (key == SDLK_LEFT || key == SDLK_RIGHT) {
            if (key == SDLK_RIGHT) {
                r->hud->map++;
                if (r->hud->map == TABLE_COUNT)
                    r->hud->map = 0;
            } else {
                if (r->hud->map == 0)
                    r->hud->map = TABLE_COUNT;
                r->hud->map--;
            }
            init_table_tiles(tiles, r->stages->tables + r->hud->map);
        } else if (key == SDLK_UP)
            r->hud->stage_b = !r->hud->stage_b;
          else if (key == SDLK_DOWN)
            r->hud->toolbox = !r->hud->toolbox;
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        size_t x = e->button.x, y = e->button.y;
        if (in_rect(map_area, x, y)) {
            table_add_hole(r->stages, r->stages->tables + r->hud->map, tiles,
                           (x - 8) / TSIZE, (y - 8) / TSIZE);
        } else {
            hud_click(r->hud, x, y);
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
    table_tiles tiles;
    SDL_Event e = {0};
    init_table_tiles(&tiles, r.stages->tables);
    SDL_SetRenderTarget(r.renderer, NULL);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        handle_event(&e, &r, &tiles);
        render_stage(r.renderer, r.sprites, r.stages, r.hud, &tiles);
        render_hud(r.renderer, r.hud, r.sprites, r.stages);
        SDL_RenderPresent(r.renderer);
    }
    resources_cleanup(&r);
    return EXIT_SUCCESS;
}
