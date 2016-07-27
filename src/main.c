#include <SDL2/SDL.h>
#include <stdio.h>
#include <integer.h>
#include <sprite.h>
#include <stage.h>
#include <render.h>
// Posix
#include <unistd.h>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 480

typedef struct resources {
    SDL_Window* window;
    SDL_Renderer* renderer;
    lb_sprites* sprites;
    lb_stages* stages;
    FILE* rom;
} resources;

void resources_cleanup(resources* res) {
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
    table_full* table = get_table(r.stages, 0);
    stage_ball* balls = get_balls(r.stages, 0);
    table_tiles tiles;
    size_t i = 0;
    SDL_Event e = {0};
    init_table_tiles(&tiles, table);
    SDL_SetRenderTarget(r.renderer, NULL);
    render_stage(r.renderer, r.sprites, table, balls, &tiles);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            i++;
            if (i >= STAGE_COUNT)
                i = 0;
            table = get_table(r.stages, i);
            balls = get_balls(r.stages, i);
            init_table_tiles(&tiles, table);
            render_stage(r.renderer, r.sprites, table, balls, &tiles);
        }
        SDL_RenderPresent(r.renderer);
    }
    resources_cleanup(&r);
    return EXIT_SUCCESS;
}
