#include <SDL2/SDL.h>
#include <sprite.h>
#include <render.h>
#include <stdio.h>
#include <stdlib.h>
#include <integer.h>

#define TILE_SIZE 8
#define BALL_SIZE 16
#define TILE_BYTE_SIZE 16
#define ROM_PPU_OFFSET 0x4010
#define PPU_LETTERS_OFFSET (0x120 * TILE_BYTE_SIZE)
#define ROM_PALETTE_BALL_OFFSET 0x2D0C
#define ROM_PALETTE_BALL_SIZE   4

typedef struct ppu_palette {
    u8 color[4];
} ppu_palette;

typedef struct rgba_palette {
    rgba_color color[4];
} rgba_palette;

rgba_palette ppu_to_rgba(ppu_palette palette) {
    rgba_palette colors;
    for (size_t i = 0; i < 4; i++)
        colors.color[i] = get_color(palette.color[i]);
    return colors;
}

SDL_Texture* create_texture(SDL_Renderer* renderer, size_t x, size_t y) {
    SDL_Texture* tte = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET, x, y);
    SDL_SetTextureBlendMode(tte, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, tte);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    return tte;
}

SDL_Texture* create_texture_tile(SDL_Renderer* renderer, FILE* rom, 
                                 rgba_palette palette) {
    SDL_Texture* tte = create_texture(renderer, TILE_SIZE, TILE_SIZE);
    u64 data1, data2;
    fread(&data1, sizeof(data1), 1, rom);
    fread(&data2, sizeof(data2), 1, rom);
    for (size_t i = 0; i < TILE_SIZE; i++)
        for (size_t j = 0; j < TILE_SIZE; j++) {
            u8 palette_index = (data1 & 1) + ((data2 & 1) << 1);
            set_render_color(renderer, palette.color[palette_index]);
            SDL_RenderDrawPoint(renderer, TILE_SIZE - j - 1, i);
            data1 >>= 1;
            data2 >>= 1;
        }
    return tte;
}

SDL_Texture* create_texture_ball(SDL_Renderer* renderer, FILE* rom,
                                 rgba_palette palette) {
    SDL_Texture* ball = create_texture(renderer, BALL_SIZE, BALL_SIZE);
    for (size_t x = 0; x < 2; x++)
        for (size_t y = 0; y < 2; y++) {
            SDL_Rect dst = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_Texture* corner = create_texture_tile(renderer, rom, palette);
            SDL_SetRenderTarget(renderer, ball);
            SDL_RenderCopy(renderer, corner, NULL, &dst);
            SDL_DestroyTexture(corner);
        }
    return ball;
}

void sprites_tiles_init(SDL_Renderer* renderer, FILE* rom, 
                       SDL_Texture** textures, size_t count) {
    const ppu_palette colors = {{0x1D, 0x30, 0x00, 0x09}};
    for (size_t i = 0; i < count; i++)
        textures[i] = create_texture_tile(renderer, rom, ppu_to_rgba(colors));
}

lb_sprites* sprites_init(SDL_Renderer* renderer, FILE* rom) {
    lb_sprites* sprites = (lb_sprites*) malloc(sizeof(lb_sprites));
    ppu_palette colors[ROM_PALETTE_BALL_SIZE];
    rgba_palette palettes[ROM_PALETTE_BALL_SIZE];
    fseek(rom, ROM_PALETTE_BALL_OFFSET, SEEK_SET);
    fread(colors, sizeof(colors), 1, rom);
    for (size_t i = 0; i < 4; i++)
        palettes[i] = ppu_to_rgba(colors[i]); 
    fseek(rom, ROM_PPU_OFFSET, SEEK_SET);
    for (size_t i = 0; i < SPRITE_BALL_COUNT; i++)
        sprites->balls[i] = create_texture_ball(renderer, rom, palettes[i >> 1]);
    fseek(rom, ROM_PPU_OFFSET + PPU_LETTERS_OFFSET, SEEK_SET);
    sprites_tiles_init(renderer, rom, sprites->letters, SPRITE_LETTER_COUNT);
    sprites_tiles_init(renderer, rom, sprites->blocks, SPRITE_BLOCK_COUNT);
    sprites_tiles_init(renderer, rom, sprites->slopes, SPRITE_SLOPE_COUNT);
    sprites_tiles_init(renderer, rom, sprites->holes, SPRITE_HOLE_COUNT);
    sprites_tiles_init(renderer, rom, sprites->moons, SPRITE_MOON_COUNT);
    return sprites;
}

void sprites_destroy(lb_sprites* sprites) {
    for (size_t i = 0; i < SPRITE_BALL_COUNT; i++)
        SDL_DestroyTexture(sprites->balls[i]);
    for (size_t i = 0; i < SPRITE_BLOCK_COUNT; i++)
        SDL_DestroyTexture(sprites->blocks[i]);
    for (size_t i = 0; i < SPRITE_SLOPE_COUNT; i++)
        SDL_DestroyTexture(sprites->slopes[i]);
    for (size_t i = 0; i < SPRITE_HOLE_COUNT; i++)
        SDL_DestroyTexture(sprites->holes[i]);
    for (size_t i = 0; i < SPRITE_MOON_COUNT; i++)
        SDL_DestroyTexture(sprites->moons[i]);
    free(sprites);
}
