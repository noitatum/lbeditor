#include <integer.h>
#include <stdlib.h>
#include <stdio.h>
#include <stage.h>
#include <string.h>

#define ROM_STAGE_TABLE_OFFSET 0x1507
#define ROM_STAGE_ORDER_OFFSET 0x4D47
#define ROM_RAM_OFFSET         0xBFF0
#define TABLE_END_BIT          0x80 
#define TABLE_POS_MASK         0x1F 
#define TYPE_HORIZONTAL        0x00
#define TYPE_VERTICAL          0x01
#define TYPE_DIAGONAL          0x02
#define TYPE_BODY              0x03
#define FLAG_BODY_SLANT        0x80
#define FLAG_BODY_BLOCK        0x40 

u8 line_type(table_line* line) {
    return (line->x >> 5) & 0x3;
}

void table_init(FILE* rom, table_full* table) {
    size_t i;
    for (i = 0; i < TABLE_MAX_LINES; i++) {
        table_line* line = table->lines + i;
        fread(line, 2, 1, rom);
        if (line_type(line) != TYPE_BODY)
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
} 

lb_stages* stages_init(FILE* rom) {
    lb_stages* stages = (lb_stages*) malloc(sizeof(lb_stages));
    fseek(rom, ROM_STAGE_ORDER_OFFSET, SEEK_SET);
    fread(stages, sizeof(stages->order) + sizeof(stages->balls), 1, rom);    
    u16 table[TABLE_COUNT];
    fseek(rom, ROM_STAGE_TABLE_OFFSET, SEEK_SET);
    fread(table, sizeof(table), 1, rom);
    for (size_t i = 0; i < TABLE_COUNT; i++) {
        fseek(rom, (size_t) table[i] - ROM_RAM_OFFSET, SEEK_SET);
        table_init(rom, stages->tables + i);
    }
    return stages;
}

void tile_table_hole(table_tiles* tiles, table_hole* hole) {
    for (size_t j = 0; j < 4; j++)
        tiles->tiles[hole->y + (j >> 1)][hole->x + (j & 1)] |= 0x10 << j;
}

void tile_table_line(table_tiles* tiles, table_line* line) {
    u8 x = line->x & TABLE_POS_MASK;
    u8 y = line->y & TABLE_POS_MASK;
    u8 type = line_type(line); 
    if (type == TYPE_BODY) {
        if (line->y & FLAG_BODY_SLANT) {
            for (size_t j = 0; j < 4; j++)
                tiles->tiles[y + (j >> 1)][x + (j & 1)] |= 3 * (4 - j);
        } else {
            if (line->y & FLAG_BODY_BLOCK)
                tiles->tiles[y][x] |= TILE_MASK_BLOCK;
            else
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
        static const u8 SLOPE_TABLE[4] = {0x03, 0x06, 0x0C, 0x09};
        u8 tile = line->y & 0x20 ? TILE_MASK_BLOCK : SLOPE_TABLE[line->y >> 6];
        if (line->end > y)
            for (size_t j = x, k = y; k <= line->end; j++, k++)
                tiles->tiles[k][j] |= tile; 
        else
            for (size_t j = x, k = y; k >= line->end; j++, k--)
                tiles->tiles[k][j] |= tile; 
    }
}

void fuse_slopes(table_tiles* tiles) {
    for (size_t j = 0; j < TABLE_MAX_Y; j++)
        for (size_t i = 0; i < TABLE_MAX_X; i++)
            if ((tiles->tiles[j][i] & TILE_MASK_BLOCK) % 3) 
                tiles->tiles[j][i] |= TILE_MASK_BLOCK;
}

void init_table_tiles(table_tiles* tiles, table_full* table) {
    memset(tiles, 0, sizeof(*tiles));
    for (size_t i = 0; i < table->line_count; i++)
        tile_table_line(tiles, table->lines + i);
    fuse_slopes(tiles);
    for (size_t i = 0; i < table->hole_count; i++)
        tile_table_hole(tiles, table->holes + i);
}
