#include <SDL2/SDL.h>
#include <stdio.h>
#include <integer.h>
#include <macros.h>
#include <sprite.h>
#include <stage.h>
#include <render.h>
// Posix
#include <unistd.h>

const char* MAIN_TITLE = "Moon Editor";

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 480

typedef struct sdl_context {
    SDL_Window* window;
    SDL_Renderer* renderer;
} sdl_context;

void context_destroy(sdl_context* c) {
    if (c->renderer)
        SDL_DestroyRenderer(c->renderer);
    if (c->window)
        SDL_DestroyWindow(c->window);
}

int context_init(sdl_context* c) {
    // Try to initialize SDL
    SDL_RET_IF_ERROR(SDL_Init(SDL_INIT_VIDEO));
    // Try to initialize the window
    c->window = SDL_CreateWindow(MAIN_TITLE, SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                 SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_RET_IF_NULL(c->window);
    // Try to initialize the renderer of the window
    c->renderer = SDL_CreateRenderer(c->window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RET_IF_NULL(c->renderer);
    SDL_SetRenderDrawBlendMode(c->renderer, SDL_BLENDMODE_BLEND);
    // FIXME: Why is this needed? 
    // First sprite won't load properly without drawing a non transparent point
    SDL_SetRenderDrawColor(c->renderer, 255, 255, 255, 255);
    SDL_RenderDrawPoint(c->renderer, 0, 0);
    return 0;
}

void show_stage(SDL_Renderer* renderer, size_t stage, lb_sprites* sprites,
                  lb_stages* stages, table_tiles* tiles) {
    size_t index = stages->order[stage] - 1;
    table_full* table = stages->tables + index % TABLE_COUNT;
    init_table_tiles(tiles, table);
    render_back(renderer, table);
    render_table(renderer, tiles, sprites); 
    render_balls(renderer, stages->balls[index], sprites);
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    RET_IF_TRUE(argc != 2, "Usage moon-editor <Lunar Ball Rom>\n");
    FILE* rom = fopen(argv[1], "r");
    RET_IF_FALSE(rom, "Couldn't open file %s\n", argv[1]);
    sdl_context context;
    if (context_init(&context)) {
        context_destroy(&context);
        return EXIT_FAILURE;
    }
    SDL_Renderer* renderer = context.renderer;
    lb_sprites* sprites = sprites_init(renderer, rom);
    lb_stages* stages = stages_init(rom);
    table_tiles tiles;
    size_t i = 0;
    SDL_Event e = {0};
    SDL_SetRenderTarget(renderer, NULL);
    show_stage(renderer, i, sprites, stages, &tiles);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            i++;
            if (i >= STAGE_COUNT)
                i = 0;
            show_stage(renderer, i, sprites, stages, &tiles);
        }
    }
    sprites_destroy(sprites);
    context_destroy(&context);
    SDL_Quit();
    fclose(rom);
    return EXIT_SUCCESS;
}
