#include <SDL2/SDL.h>
#include <stdio.h>
#include <integer.h>
#include <sprite.h>
#include <stage.h>
#include <render.h>
#include <hud.h>
// Posix
#include <unistd.h>

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
    return SDL_CreateWindow("Moon Editor", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    table_full* table = r.stages->tables;
    size_t i = 0, st = table->stage_a;
    stage_ball* balls = get_balls(r.stages, st);
    table_tiles tiles;
    SDL_Event e = {0};
    init_table_tiles(&tiles, table);
    SDL_SetRenderTarget(r.renderer, NULL);
    render_stage(r.renderer, r.sprites, table, balls, &tiles);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            if (key == SDLK_LEFT || key == SDLK_RIGHT) {
                if (key == SDLK_RIGHT) {
                    i++;
                    if (i == TABLE_COUNT)
                        i = 0;
                } else {
                    if (i == 0)
                        i = TABLE_COUNT;
                    i--;
                }
                table = r.stages->tables + i; 
                st = table->stage_a;
                balls = get_balls(r.stages, st);
                init_table_tiles(&tiles, table);
                render_stage(r.renderer, r.sprites, table, balls, &tiles);
            }
            if (key == SDLK_UP) {
                st = st == table->stage_a ? table->stage_b : table->stage_a;
                balls = get_balls(r.stages, st);
                render_stage(r.renderer, r.sprites, table, balls, &tiles);
            }
            if (key == SDLK_DOWN) 
                r.hud->toolbox = !r.hud->toolbox;
        } 
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.y / TSIZE > TABLE_MIN_Y) {
                table_add_hole(r.stages, table, &tiles, 
                               (e.button.x - 8) / TSIZE, (e.button.y - 8) / TSIZE);
                render_stage(r.renderer, r.sprites, table, balls, &tiles);
            } else {
                hud_click(r.hud, e.button.x, e.button.y);
            }
        }
        render_hud(r.renderer, r.hud, r.sprites, r.stages, i);
        SDL_RenderPresent(r.renderer);
    }
    resources_cleanup(&r);
    return EXIT_SUCCESS;
}
