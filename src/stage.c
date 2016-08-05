#include <integer.h>
#include <stdlib.h>
#include <stdio.h>
#include <stage.h>
#include <string.h>
#include <hud.h>

#define TABLE_END_BIT    0x80
#define TABLE_POS_MASK   0x1F

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
        size_t order = stages->order[i] - 1;
        size_t table = order % TABLE_COUNT;
        size_t stage_b = order / TABLE_COUNT;
        stages->tables[table].stages[stage_b] = i + 1;
        stages->tables[table].balls[stage_b] = stages->balls[order];
    }
    return stages;
}

void table_write(table_full* table, FILE* rom) {
    if (table->line_count) {
        table->lines[table->line_count - 1].x |= TABLE_END_BIT;
        for (size_t i = 0; i < table->line_count; i++) {
            table_line* line = table->lines + i;
            fwrite(line, ((line->x & TYPE_MASK) == TYPE_BODY) ? 2 : 3, 1, rom);
        }
        table->lines[table->line_count - 1].x ^= TABLE_END_BIT;
    }
    if (table->back_count) {
        table->backs[table->back_count - 1].x1 |= TABLE_END_BIT;
        fwrite(table->backs, sizeof(table_back), table->back_count, rom);
        table->backs[table->back_count - 1].x1 ^= TABLE_END_BIT;
    }
    if (table->hole_count) {
        table->holes[table->hole_count - 1].x |= TABLE_END_BIT;
        fwrite(table->holes, sizeof(table_hole), table->hole_count, rom);
        table->holes[table->hole_count - 1].x ^= TABLE_END_BIT;
    }
}

void stages_write(lb_stages* stages, FILE* rom) {
    u16 table[TABLE_COUNT];
    fseek(rom, ROM_STAGE_ORDER_OFFSET, SEEK_SET);
    fwrite(stages->order, sizeof(stages->order) + sizeof(stages->balls), 1, rom);
    fseek(rom, ROM_MAP_DATA_START, SEEK_SET);
    for (size_t i = 0, bytes = ROM_MAP_DATA_BYTES; i < TABLE_COUNT; i++) {
        if (stages->tables[i].byte_count > bytes)
            break;
        bytes -= stages->tables[i].byte_count;
        table[i] = ftell(rom) + ROM_RAM_OFFSET;
        table_write(stages->tables + i, rom);
    }
    fseek(rom, ROM_STAGE_TABLE_OFFSET, SEEK_SET);
    fwrite(table, sizeof(table), 1, rom);
}

void tile_table_wall(table_tiles* tiles, size_t x, size_t y, u8 wall,
                     u8* backup, size_t n, size_t reverse) {
    u8* tile = tiles->walls[y] + x;
    if (backup) {
        if (reverse) {
            *tile = backup[n];
            return;
        }
        backup[n] = *tile;
    }
    *tile |= wall;
    if (*tile % 3)
        *tile |= TILE_BLOCK;
}

void tile_table_line(table_tiles* tiles, const table_line* line, u8* backup,
                     size_t reverse) {
    u8 x = line->x & TABLE_POS_MASK;
    u8 y = line->y & TABLE_POS_MASK;
    u8 type = line->x & TYPE_MASK;
    if (type == TYPE_BODY) {
        if (line->y & FLAG_BODY_SLANT) {
            static const u8 slope_table[4] = {0x0C, 0x09, 0x06, 0x03};
            for (size_t j = 0; j < 4; j++)
                tile_table_wall(tiles, x + (j & 1), y + (j >> 1),
                                slope_table[j], backup, j, reverse);
        } else if (line->y & FLAG_BODY_BLOCK) {
            tile_table_wall(tiles, x, y, TILE_BLOCK, backup, 0, reverse);
        } else {
            for (size_t j = 0; j < 4; j++)
                tile_table_wall(tiles, x + (j & 1), y + (j >> 1),
                                TILE_BLOCK, backup, j, reverse);
        }
    } else if (type == TYPE_HORIZONTAL) {
        for (size_t j = x; j <= line->end; j++)
            tile_table_wall(tiles, j, y, TILE_BLOCK, backup, j - x, reverse);
    } else if (type == TYPE_VERTICAL) {
        for (size_t j = y; j <= line->end; j++)
            tile_table_wall(tiles, x, j, TILE_BLOCK, backup, j - y, reverse);
    } else {
        static const u8 slope_table[4] = {0x03, 0x06, 0x0C, 0x09};
        u8 tile = slope_table[line->y >> 6];
        if (line->y & FLAG_LINE_BLOCK)
            tile = TILE_BLOCK;
        if (line->end > y)
            for (size_t j = x, k = y; k <= line->end; j++, k++)
                tile_table_wall(tiles, j, k, tile, backup, j - x, reverse);
        else
            for (size_t j = x, k = y; k >= line->end; j++, k--)
                tile_table_wall(tiles, j, k, tile, backup, j - x, reverse);
    }
}

int table_add_line(table_full* table, table_tiles* tiles, size_t x1, size_t y1,
                    size_t x2, size_t y2, size_t tool, u8* backup) {
    if (table->line_count == TABLE_MAX_LINES)
        return -1;
    ssize_t x = x1, y = y1, end = y2;
    size_t wall = 0, type = TYPE_DIAGONAL, bytes = sizeof(table_line);
    if ((x1 == x2 && y1 == y2 && tool == TOOL_BLOCK) ||
        tool == TOOL_SLANT || tool == TOOL_SQUARE) {
        x = x2, y = y2, wall = FLAG_BODY_BLOCK, type = TYPE_BODY;
        if (tool == TOOL_SLANT)
            wall = FLAG_BODY_SLANT;
        else if (tool == TOOL_SQUARE)
            wall = FLAG_BODY_SQUARE;
        bytes--;
    } else if (y1 == y2 && tool == TOOL_BLOCK) {
        end = x2, type = TYPE_HORIZONTAL;
        if (x1 > x2)
            x = x2, end = x1;
    } else if (x1 == x2 && tool == TOOL_BLOCK) {
        type = TYPE_VERTICAL;
        if (y1 > y2)
            y = y2, end = y1;
    } else {
        wall = FLAG_LINE_BLOCK;
        if (tool != TOOL_BLOCK)
            wall = tool << 6;
        // Revert line if necessary
        if (x1 > x2)
            x = x1 - (y1 > y2 ? y1 - y2 : y2 - y1), y = y2, end = y1;
        // Check bounds and correct if necessary
        ssize_t sign = (end > y) * 2 - 1;
        if (x < MAP_MIN_X) {
            y += sign * (MAP_MIN_X - x);
            x = MAP_MIN_X;
        } else if (x + sign * (end - y) > MAP_MAX_X)
            end = y + sign * (MAP_MAX_X - x);
    }
    table_line* line = table->lines + table->line_count;
    *line = (table_line) {x | type, y | wall, end};
    tile_table_line(tiles, line, backup, 0);
    table->byte_count += bytes;
    table->line_count++;
    return 0;
}

void tile_table_hole(table_tiles* tiles, table_hole* hole,
                     u8* backup, size_t reverse) {
    for (size_t j = 0; j < 4; j++) {
        u8* tile = tiles->holes[hole->y + (j >> 1)] + hole->x + (j & 1);
        if (backup) {
            if (reverse) {
                *tile = backup[j];
                continue;
            }
            backup[j] = *tile;
        }
        *tile |= 1 << j;
    }
}

int table_add_hole(table_full* table, table_tiles* tiles, size_t x, size_t y,
                   u8* backup) {
    if (table->hole_count == TABLE_MAX_HOLES)
        return -1;
    table_hole* hole = table->holes + table->hole_count;
    *hole = (table_hole) {x, y};
    tile_table_hole(tiles, hole, backup, 0);
    table->hole_count++;
    table->byte_count += sizeof(table_hole);
    return 0;
}

int table_add_back(table_full* table, table_tiles* tiles,
                   size_t x1, size_t y1, size_t x2, size_t y2) {
    if (table->back_count == TABLE_MAX_BACKS)
        return -1;
    size_t tx1 = x1, tx2 = x2, ty1 = y1, ty2 = y2;
    if (x1 > x2)
        tx1 = x2, tx2 = x1;
    if (y1 > y2)
        ty1 = y2, ty2 = y1;
    table->backs[table->back_count] = (table_back) {tx1, ty1, tx2, ty2};
    tiles->backs[tiles->back_count] = (table_back) {tx1, ty1, tx2, ty2};
    tiles->back_count++;
    table->back_count++;
    table->byte_count += sizeof(table_back);
    return 0;
}

void table_remove_line(table_full* table, table_tiles* tiles, u8* backup) {
    table->line_count--;
    table->byte_count -= sizeof(table_line) -
        ((table->lines[table->line_count].x & TYPE_BODY) == TYPE_BODY);
    tile_table_line(tiles, table->lines + table->line_count, backup, 1);
}

void table_remove_hole(table_full* table, table_tiles* tiles, u8* backup) {
    table->hole_count--;
    table->byte_count -= sizeof(table_hole);
    tile_table_hole(tiles, table->holes + table->hole_count, backup, 1);
}

void table_remove_back(table_full* table, table_tiles* tiles) {
    table->back_count--;
    table->byte_count -= sizeof(table_back);
    tiles->back_count--;
}

void table_clear(table_full* table, table_tiles* tiles) {
    uintptr_t start = (uintptr_t) &table->byte_count;
    uintptr_t end = (uintptr_t) (table + 1);
    memset((void*) start, 0, end - start);
    memset(tiles, 0, sizeof(table_tiles));
}

void tile_table_lines(table_tiles* tiles, const table_line* lines,
                      size_t count) {
    for (size_t i = 0; i < count; i++)
        tile_table_line(tiles, lines + i, NULL, 0);
}

void init_table_tiles(table_tiles* tiles, table_full* table) {
    memset(tiles, 0, sizeof(*tiles));
    tile_table_lines(tiles, table->lines, table->line_count);
    for (size_t i = 0; i < table->hole_count; i++)
        tile_table_hole(tiles, table->holes + i, NULL, 0);
    memcpy(tiles->backs, table->backs, sizeof(table->backs));
    tiles->back_count = table->back_count;
}
