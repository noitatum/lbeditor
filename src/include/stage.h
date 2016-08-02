#pragma once

#include <stdio.h>
#include <integer.h>

#define GRID_WIDTH       0x20
#define GRID_HEIGHT      0x1E
#define STAGE_COUNT      60
#define TABLE_COUNT      30

#define TABLE_MAX_LINES  256
#define TABLE_MAX_BACKS  32
#define TABLE_MAX_HOLES  256
#define BALL_COUNT       8

#define TILE_BLOCK       0x0F
#define SLOPE_NW         0x00
#define SLOPE_NE         0x40
#define SLOPE_SE         0x80
#define SLOPE_SW         0xC0
#define TYPE_HORIZONTAL  0x00
#define TYPE_VERTICAL    0x20
#define TYPE_DIAGONAL    0x40
#define TYPE_BODY        0x60
#define TYPE_MASK        0x60
#define FLAG_BODY_SLANT  0x80
#define FLAG_BODY_BLOCK  0x40
#define FLAG_BODY_SQUARE 0x00
#define FLAG_LINE_BLOCK  0x20

#define ROM_STAGE_TABLE_OFFSET 0x1507
#define ROM_STAGE_ORDER_OFFSET 0x4D47
#define ROM_RAM_OFFSET         0xBFF0

typedef struct stage_ball {
    u8 x, y;
} stage_ball;

typedef struct table_line {
    u8 x, y, end;
} table_line;

typedef struct table_back {
    u8 x1, y1, x2, y2;
} table_back;

typedef struct table_hole {
    u8 x, y;
} table_hole;

typedef struct table_full {
    size_t stage_a, stage_b;
    size_t byte_count, line_count, back_count, hole_count;
    table_line lines[TABLE_MAX_LINES];
    table_back backs[TABLE_MAX_BACKS];
    table_hole holes[TABLE_MAX_HOLES];
} table_full;

typedef struct lb_stages {
    size_t     byte_count;
    u8         order[STAGE_COUNT];
    stage_ball balls[STAGE_COUNT][BALL_COUNT];
    table_full tables[TABLE_COUNT];
} lb_stages;

typedef struct table_tiles {
    u8 walls[GRID_HEIGHT][GRID_WIDTH];
    u8 holes[GRID_HEIGHT][GRID_WIDTH];
    table_back backs[TABLE_MAX_BACKS];
    size_t back_count;
} table_tiles;

lb_stages* stages_init(FILE* rom);
void tile_table_lines(table_tiles* tiles, const table_line* lines, size_t count);
void tile_table_line(table_tiles* tiles, const table_line* line, u8* backup,
                     size_t reverse);
int table_add_line(table_full* table, table_tiles* tiles, size_t x1, size_t y1,
                    size_t x2, size_t y2, size_t tool, u8* backup);
int table_add_hole(table_full* table, table_tiles* tiles, size_t x, size_t y,
                   u8* backup);
int table_add_back(table_full* table, table_tiles* tiles,
                   size_t x1, size_t y1, size_t x2, size_t y2);
void table_remove_line(table_full* table, table_tiles* tiles, u8* backup);
void table_remove_hole(table_full* table, table_tiles* tiles, u8* backup);
void table_remove_back(table_full* table, table_tiles* tiles);
void init_table_tiles(table_tiles* tiles, table_full* table);
