#pragma once

#include <stdio.h>
#include <integer.h>

#define STAGE_COUNT     60
#define TABLE_COUNT     30
#define TABLE_MAX_LINES 256
#define TABLE_MAX_BACKS 32
#define TABLE_MAX_HOLES 256
#define BALL_COUNT      8
#define TABLE_MAX_X     0x20
#define TABLE_MAX_Y     0x20
#define TILE_MASK_BLOCK 0x0F

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
    size_t line_count, back_count, hole_count;
    table_line lines[TABLE_MAX_LINES];
    table_back backs[TABLE_MAX_BACKS];
    table_hole holes[TABLE_MAX_HOLES];
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
