#include <integer.h>
#include <stdlib.h>
#include <stdio.h>
#include <stage.h>
#include <string.h>

#define TABLE_END_BIT   0x80 
#define TABLE_POS_MASK  0x1F 
#define FLAG_BODY_SLANT 0x80
#define FLAG_BODY_BLOCK 0x40 
#define HOLE_BIT        0x10

void table_init(FILE* rom, table_full* table) {
    size_t i;
    size_t start = ftell(rom);
    for (i = 0; i < TABLE_MAX_LINES; i++) {
        table_line* line = table->lines + i;
        fread(line, 2, 1, rom);
        if ((line->x & TYPE_MASK) != TYPE_BODY)
            fread(&(line->end), 1, 1, rom);
        if (line->x & TABLE_END_BIT) {
            line->x ^= TABLE_END_BIT;
            i++;
            break;
        }
    }
    table->line_count = i;
    for (i = 0; i < TABLE_MAX_BACKS; i++) {
        fread(table->backs + i, sizeof(table_back), 1, rom);
        if (table->backs[i].x1 & TABLE_END_BIT) {
            table->backs[i].x1 ^= TABLE_END_BIT;
            i++;
            break;
        }
    }
    table->back_count = i;
    for (i = 0; i < TABLE_MAX_HOLES; i++) {
        fread(table->holes + i, sizeof(table_hole), 1, rom);
        if (table->holes[i].x & TABLE_END_BIT) {
            table->holes[i].x ^= TABLE_END_BIT;
            i++;
            break;
        }
    }
    table->hole_count = i;
    table->byte_count = ftell(rom) - start; 
} 

lb_stages* stages_init(FILE* rom) {
    lb_stages* stages = (lb_stages*) malloc(sizeof(lb_stages));
    fseek(rom, ROM_STAGE_ORDER_OFFSET, SEEK_SET);
    fread(stages->order, sizeof(stages->order) + sizeof(stages->balls), 1, rom);
    u16 table[TABLE_COUNT];
    fseek(rom, ROM_STAGE_TABLE_OFFSET, SEEK_SET);
    fread(table, sizeof(table), 1, rom);
    stages->byte_count = 0;
    for (size_t i = 0; i < TABLE_COUNT; i++) {
        fseek(rom, (size_t) table[i] - ROM_RAM_OFFSET, SEEK_SET);
        table_init(rom, stages->tables + i);
        stages->byte_count += stages->tables[i].byte_count;
    }
    for (size_t i = 0; i < STAGE_COUNT; i++) {
        size_t table = stages->order[i] - 1;
        if (table < TABLE_COUNT)
            stages->tables[table].stage_a = i;
        else
            stages->tables[table - TABLE_COUNT].stage_b = i;
    }
    return stages;
}

void tile_table_hole(table_tiles* tiles, table_hole* hole) {
    for (size_t j = 0; j < 4; j++)
        tiles->tiles[hole->y + (j >> 1)][hole->x + (j & 1)] |= HOLE_BIT << j;
}

void tile_table_slope(table_tiles* tiles, size_t x, size_t y, u8 slope) {
    tiles->tiles[y][x] |= slope;
    if ((tiles->tiles[y][x] & TILE_MASK_BLOCK) % 3) 
        tiles->tiles[y][x] |= TILE_MASK_BLOCK;
}

void tile_table_line(table_tiles* tiles, const table_line* line) {
    static const u8 slope_table[4] = {0x03, 0x06, 0x0C, 0x09};
    u8 x = line->x & TABLE_POS_MASK;
    u8 y = line->y & TABLE_POS_MASK;
    u8 type = line->x & TYPE_MASK; 
    if (type == TYPE_BODY) {
        if (line->y & FLAG_BODY_SLANT) {
            tile_table_slope(tiles, x + 1, y + 1, slope_table[0]);
            tile_table_slope(tiles, x + 0, y + 1, slope_table[1]);
            tile_table_slope(tiles, x + 0, y + 0, slope_table[2]);
            tile_table_slope(tiles, x + 1, y + 0, slope_table[3]);
        } else if (line->y & FLAG_BODY_BLOCK) {
            tiles->tiles[y][x] |= TILE_MASK_BLOCK;
        } else {
            for (size_t j = 0; j < 4; j++)
                tiles->tiles[y + (j >> 1)][x + (j & 1)] |= TILE_MASK_BLOCK;
        }
    } else if (type == TYPE_HORIZONTAL) {
        for (size_t j = x; j <= line->end; j++)
            tiles->tiles[y][j] |= TILE_MASK_BLOCK;
    } else if (type == TYPE_VERTICAL) {
        for (size_t j = y; j <= line->end; j++)
            tiles->tiles[j][x] |= TILE_MASK_BLOCK;
    } else {
        u8 tile = line->y & 0x20 ? TILE_MASK_BLOCK : slope_table[line->y >> 6];
        if (line->end > y)
            for (size_t j = x, k = y; k <= line->end; j++, k++)
                tile_table_slope(tiles, j, k, tile);
        else
            for (size_t j = x, k = y; k >= line->end; j++, k--)
                tile_table_slope(tiles, j, k, tile);
    }
}

void tile_table_lines(table_tiles* tiles, const table_line* lines, size_t count) {
    for (size_t i = 0; i < count; i++)
        tile_table_line(tiles, lines + i);
}

void init_table_tiles(table_tiles* tiles, table_full* table) {
    memset(tiles, 0, sizeof(*tiles));
    tile_table_lines(tiles, table->lines, table->line_count);
    for (size_t i = 0; i < table->hole_count; i++)
        tile_table_hole(tiles, table->holes + i);
}

int table_add_hole(lb_stages* stages, table_full* table, table_tiles* tiles, 
                   size_t x, size_t y) {
    if (table->hole_count == TABLE_MAX_HOLES)
        return -1;
    if (tiles->tiles[y][x] & HOLE_BIT)
        return 0;
    table_hole* hole = table->holes + table->hole_count; 
    hole->x = x;
    hole->y = y;
    tile_table_hole(tiles, hole);
    table->hole_count++;
    table->byte_count += sizeof(table_hole);
    stages->byte_count += sizeof(table_hole);
    return 0;
}
