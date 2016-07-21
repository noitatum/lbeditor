#pragma once

#include <stdio.h>
#include <integer.h>

#define STAGE_COUNT     60
#define TABLE_COUNT     30
#define TABLE_MAX_LINES 256
#define TABLE_MAX_BACKS 256
#define TABLE_MAX_HOLES 256
#define BALL_COUNT      8
#define TABLE_MAX_X     0x20
#define TABLE_MAX_Y     0x20

#define TILE_FLAG_BLOCK        0x10
#define TILE_FLAG_HOLE         0x20
#define TILE_FLAG_BACK         0x40
#define TILE_SLOPE_SE          0x00
#define TILE_SLOPE_SW          0x01
#define TILE_SLOPE_NW          0x02
#define TILE_SLOPE_NE          0x03
#define TILE_BLOCK             0x04

typedef struct position {
    u8 x, y;
} position;

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
    table_line lines[TABLE_MAX_LINES];
    table_back backs[TABLE_MAX_BACKS];
    table_hole holes[TABLE_MAX_HOLES];
    size_t line_count, back_count, hole_count;
} table_full;

typedef struct lb_stages {
    u8         order[STAGE_COUNT];
    position   balls[STAGE_COUNT][BALL_COUNT]; 
    table_full tables[TABLE_COUNT];
} lb_stages; 

typedef struct table_tiles {
    u8 tiles[TABLE_MAX_Y][TABLE_MAX_X];
} table_tiles;

lb_stages* stages_init(FILE* rom);
void init_table_tiles(table_tiles* tiles, table_full* table);
