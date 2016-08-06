#pragma once

#include <stdio.h>
#include <integer.h>

#define GRID_WIDTH          0x20
#define GRID_HEIGHT         0x1E
#define MAP_MIN_X           0x03
#define MAP_MAX_X           0x1C
#define MAP_MIN_Y           0x08
#define MAP_MAX_Y           0x1A
#define STAGE_COUNT         60
#define MAP_COUNT           30

#define MAP_MAX_LINES       255
#define MAP_MAX_BACKS       32
#define MAP_MAX_HOLES       255
#define BALL_COUNT          8

#define SLOPE_NW            0x00
#define SLOPE_NE            0x40
#define SLOPE_SE            0x80
#define SLOPE_SW            0xC0
#define TYPE_HORIZONTAL     0x00
#define TYPE_VERTICAL       0x20
#define TYPE_DIAGONAL       0x40
#define TYPE_BODY           0x60
#define TYPE_MASK           0x60
#define FLAG_BODY_SLANT     0x80
#define FLAG_BODY_BLOCK     0x40
#define FLAG_BODY_SQUARE    0x00
#define FLAG_LINE_BLOCK     0x20
#define TILE_BLOCK          0x04
#define TILE_BLOCK_FLAGS    0x0F
#define LINE_POS_MASK       0x1F

#define ROM_MAP_DATA_BYTES      0x0AC5
#define ROM_MAP_DATA_START      0x1566
#define ROM_MAP_TABLE_OFFSET    0x1507
#define ROM_STAGE_ORDER_OFFSET  0x4D47
#define ROM_RAM_OFFSET          0xBFF0

typedef struct stage_ball {
    u8 y, x;
} stage_ball;

typedef struct map_line {
    u8 x, y, end;
} map_line;

typedef struct map_back {
    u8 x1, y1, x2, y2;
} map_back;

typedef struct map_hole {
    u8 x, y;
} map_hole;

typedef struct map_full {
    size_t stages[2];
    stage_ball* balls[2];
    size_t byte_count, line_count, back_count, hole_count;
    map_line lines[MAP_MAX_LINES];
    map_back backs[MAP_MAX_BACKS];
    map_hole holes[MAP_MAX_HOLES];
} map_full;

typedef struct lb_stages {
    size_t byte_count;
    u8 order[STAGE_COUNT];
    stage_ball balls[STAGE_COUNT][BALL_COUNT];
    map_full maps[MAP_COUNT];
} lb_stages;

typedef struct map_tile {
    u8 wall_count[5];
    u8 wall_flags;
    u8 hole_flags;
    u8 padding;
} map_tile;

typedef struct map_tiles {
    map_tile tiles[GRID_HEIGHT][GRID_WIDTH];
} map_tiles;

lb_stages* stages_init(FILE* rom);
void stages_write(lb_stages* stages, FILE* rom);
void tile_map_lines(map_tiles* tiles, const map_line* lines, size_t count);
void tile_map_line(map_tiles* tiles, const map_line* line, int sign);
void map_add_line(map_full* map, map_tiles* tiles, const map_line* line,
                  size_t index);
int map_add_line_action(map_full* map, map_tiles* tiles, size_t x1, size_t y1,
                        size_t x2, size_t y2, size_t tool);
void map_add_hole(map_full* map, map_tiles* tiles, map_hole* hole,
                  size_t index);
int map_add_hole_action(map_full* map, map_tiles* tiles, size_t x, size_t y);
void map_add_back(map_full* map, map_back* back, size_t index);
int map_add_back_action(map_full* map, size_t x1, size_t y1,
                        size_t x2, size_t y2);
void map_remove_line(map_full* map, map_tiles* tiles, size_t index);
void map_remove_hole(map_full* map, map_tiles* tiles, size_t index);
void map_remove_back(map_full* map, size_t index);
int map_find_line(map_full* map, size_t lx, size_t ly);
int map_find_hole(map_full* map, size_t hx, size_t hy);
int map_find_back(map_full* map, size_t bx, size_t by);
void map_clear(map_full* map, map_tiles* tiles);
void init_map_tiles(map_tiles* tiles, map_full* map);
