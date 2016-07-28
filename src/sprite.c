#include <SDL2/SDL.h>
#include <sprite.h>
#include <render.h>
#include <stdio.h>
#include <stdlib.h>
#include <integer.h>

#define TILE_SIZE 8
#define BIG_SIZE 16
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

void init_big_tile_corner(SDL_Renderer* renderer, FILE* rom, SDL_Texture* big,
                          rgba_palette palette, size_t x, size_t y) {
    SDL_Rect dst = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_Texture* corner = create_texture_tile(renderer, rom, palette);
    SDL_SetRenderTarget(renderer, big);
    SDL_RenderCopy(renderer, corner, NULL, &dst);
    SDL_DestroyTexture(corner);
}

SDL_Texture* create_texture_crater(SDL_Renderer* renderer, FILE* rom,
                                   rgba_palette palette) {
    SDL_Texture* crater = create_texture(renderer, BIG_SIZE, BIG_SIZE);
    init_big_tile_corner(renderer, rom, crater, palette, 1, 1);
    init_big_tile_corner(renderer, rom, crater, palette, 0, 1);
    init_big_tile_corner(renderer, rom, crater, palette, 0, 0);
    init_big_tile_corner(renderer, rom, crater, palette, 1, 0);
    return crater;
}

SDL_Texture* create_texture_ball(SDL_Renderer* renderer, FILE* rom,
                                 rgba_palette palette) {
    SDL_Texture* ball = create_texture(renderer, BIG_SIZE, BIG_SIZE);
    for (size_t x = 0; x < 2; x++)
        for (size_t y = 0; y < 2; y++)
            init_big_tile_corner(renderer, rom, ball, palette, x, y); 
    return ball;
}

void sprites_tiles_init(SDL_Renderer* renderer, FILE* rom, rgba_palette colors,
                       SDL_Texture** textures, size_t count) {
    for (size_t i = 0; i < count; i++)
        textures[i] = create_texture_tile(renderer, rom, colors);
}

lb_sprites* sprites_init(SDL_Renderer* renderer, FILE* rom) {
    static const ppu_palette TILE_COLORS = {{0x3F, 0x30, 0x00, 0x3F}};
    lb_sprites* spr = (lb_sprites*) malloc(sizeof(lb_sprites));
    ppu_palette colors[ROM_PALETTE_BALL_SIZE];
    rgba_palette pals[ROM_PALETTE_BALL_SIZE];
    fseek(rom, ROM_PALETTE_BALL_OFFSET, SEEK_SET);
    fread(colors, sizeof(colors), 1, rom);
    for (size_t i = 0; i < 4; i++)
        pals[i] = ppu_to_rgba(colors[i]); 
    fseek(rom, ROM_PPU_OFFSET, SEEK_SET);
    for (size_t i = 0; i < SPRITE_BALL_COUNT; i++)
        spr->balls[i] = create_texture_ball(renderer, rom, pals[i >> 1]);
    fseek(rom, ROM_PPU_OFFSET + PPU_LETTERS_OFFSET, SEEK_SET);
    rgba_palette rgba = ppu_to_rgba(TILE_COLORS);
    sprites_tiles_init(renderer, rom, rgba, spr->letters, SPRITE_LETTER_COUNT);
    rgba.color[0] = get_color(0x1D);
    sprites_tiles_init(renderer, rom, rgba, spr->blocks, SPRITE_ALL_COUNT);
    spr->crater = create_texture_crater(renderer, rom, rgba); 
    sprites_tiles_init(renderer, rom, rgba, spr->dusts, SPRITE_DUST_COUNT);
    return spr;
}

void sprites_destroy(lb_sprites* sprites) {
    for (size_t i = 0; i < sizeof(sprites) / sizeof(SDL_Texture*); i++)
        SDL_DestroyTexture(((SDL_Texture**) sprites)[i]);
    free(sprites);
}
