#include <SDL2/SDL.h>
#include <stdio.h>
#include <integer.h>
#include <sprite.h>
#include <stage.h>
#include <render.h>
#include <hud.h>
#include <action.h>

const SDL_Rect map_area = TILE_AREA_RECT(MAP_MIN_X, MAP_MIN_Y,
                          MAP_MAX_X - MAP_MIN_X + 1, MAP_MAX_Y - MAP_MIN_Y + 1);

typedef struct resources {
    FILE* rom;
    SDL_Window* window;
    lb_render* render;
    lb_sprites* sprites;
    lb_stages* stages;
    lb_hud* hud;
    history_table* history;
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
    if (res->render)
        render_destroy(res->render);
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
    if (!(r->render = render_init(r->window)))
        return -1;
    if (!(r->sprites = sprites_init(r->render->renderer, rom)))
        return -1;
    if (!(r->stages = stages_init(rom)))
        return -1;
    if (!(r->hud = hud_init(r->render->renderer, r->sprites)))
        return -1;
    if (!(r->history = history_init()))
        return -1;
    return 0;
}

void exit_error(resources* res) {
    resources_destroy(res);
    fprintf(stderr, "SDL Failure: %s", SDL_GetError());
    exit(EXIT_FAILURE);
}

void handle_event(SDL_Event* e, resources* r, map_tiles* tiles,
                  size_t* active_action) {
    map_full* map = r->stages->maps + r->hud->map;
    history* history = r->history->table + r->hud->map;
    size_t* inv = &r->render->invalid_layers;
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode key = e->key.keysym.sym;
        hud_key(r->hud, key, inv);
        // If table changed retile
        if (map != r->stages->maps + r->hud->map) {
            init_map_tiles(tiles, r->stages->maps + r->hud->map);
            *active_action = 0;
        }
        if (key == SDLK_u) {
            // Undo action
            history_undo(history, map, tiles, inv);
            *active_action = 0;
        } else if (key == SDLK_DELETE) {
            // Delete map
            map_clear(map, tiles);
            history_clear(history);
            *inv |= LAYER_FLAGS_ALL & ~(1 << LAYER_DUST);
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        size_t x = e->button.x, y = e->button.y;
        size_t stage_b = r->hud->stage_b, tool = hud_tool(r->hud);
        if (e->button.button == SDL_BUTTON_LEFT) {
            if (in_rect(&map_area, x, y)) {
                history_do(history, map, tiles, stage_b, tool, x, y, 0, inv);
                *active_action = 1;
            } else {
                hud_click(r->hud, x, y, inv);
            }
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            if (in_rect(&map_area, x, y)) {
                history_do(history, map, tiles, stage_b, tool, x, y, 1, inv);
                *active_action = 1;
            }
        } else if (e->button.button == SDL_BUTTON_MIDDLE) {
            history_undo(history, map, tiles, inv);
            *active_action = 0;
        }
    } else if (e->type == SDL_MOUSEMOTION) {
        size_t x = e->motion.x, y = e->motion.y;
        if (*active_action && in_rect(&map_area, x, y))
            history_redo(history, map, tiles, x, y, inv);
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        if (e->button.button == SDL_BUTTON_LEFT ||
            e->button.button == SDL_BUTTON_RIGHT)
            *active_action = 0;
    }
}

FILE* create_output_file(FILE* src) {
    FILE* dst = fopen("output.nes", "rb+");
    // Make a copy only if the file doesn't exist
    // This also fixes the case where src is output.nes
    if (!dst) {
        u8 buffer[1024];
        size_t read;
        dst = fopen("output.nes", "wb");
        fseek(src, 0, SEEK_SET);
        while ((read = fread(buffer, 1, 1024, src)))
            fwrite(buffer, 1, read, dst);
        fseek(dst, 0, SEEK_SET);
    }
    return dst;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
       fprintf(stderr, "Usage moon <Lunar Ball Rom>\n");
       return EXIT_FAILURE;
    }
    FILE* rom;
    resources r;
    if (!(rom = fopen(argv[1], "rb"))) {
        fprintf(stderr, "Couldn't open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (resources_init(&r, rom))
        exit_error(&r);
    size_t active_action = 0;
    map_tiles tiles;
    SDL_Event e = {0};
    init_map_tiles(&tiles, r.stages->maps);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        handle_event(&e, &r, &tiles, &active_action);
        map_full* map = r.stages->maps + r.hud->map;
        render_invalid(r.render, r.sprites, map, r.hud, &tiles);
        render_present(r.render);
    }
    stages_write(r.stages, create_output_file(r.rom));
    resources_destroy(&r);
    return EXIT_SUCCESS;
}
