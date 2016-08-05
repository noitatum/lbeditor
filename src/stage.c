#include <integer.h>
#include <stdlib.h>
#include <stdio.h>
#include <stage.h>
#include <string.h>
#include <hud.h>

#define MAP_END_BIT    0x80
#define MAP_POS_MASK   0x1F

void map_init(FILE* rom, map_full* map) {
    size_t i;
    size_t start = ftell(rom);
    for (i = 0; i < MAP_MAX_LINES; i++) {
        map_line* line = map->lines + i;
        fread(line, 2, 1, rom);
        if ((line->x & TYPE_MASK) != TYPE_BODY)
            fread(&(line->end), 1, 1, rom);
        if (line->x & MAP_END_BIT) {
            line->x ^= MAP_END_BIT;
            i++;
            break;
        }
    }
    map->line_count = i;
    for (i = 0; i < MAP_MAX_BACKS; i++) {
        fread(map->backs + i, sizeof(map_back), 1, rom);
        if (map->backs[i].x1 & MAP_END_BIT) {
            map->backs[i].x1 ^= MAP_END_BIT;
            i++;
            break;
        }
    }
    map->back_count = i;
    for (i = 0; i < MAP_MAX_HOLES; i++) {
        fread(map->holes + i, sizeof(map_hole), 1, rom);
        if (map->holes[i].x & MAP_END_BIT) {
            map->holes[i].x ^= MAP_END_BIT;
            i++;
            break;
        }
    }
    map->hole_count = i;
    map->byte_count = ftell(rom) - start;
}

lb_stages* stages_init(FILE* rom) {
    lb_stages* stages = (lb_stages*) malloc(sizeof(lb_stages));
    fseek(rom, ROM_STAGE_ORDER_OFFSET, SEEK_SET);
    fread(stages->order, sizeof(stages->order) + sizeof(stages->balls), 1, rom);
    u16 map_table[MAP_COUNT];
    fseek(rom, ROM_MAP_TABLE_OFFSET, SEEK_SET);
    fread(map_table, sizeof(map_table), 1, rom);
    stages->byte_count = 0;
    for (size_t i = 0; i < MAP_COUNT; i++) {
        fseek(rom, (size_t) map_table[i] - ROM_RAM_OFFSET, SEEK_SET);
        map_init(rom, stages->maps + i);
        stages->byte_count += stages->maps[i].byte_count;
    }
    for (size_t i = 0; i < STAGE_COUNT; i++) {
        size_t order = stages->order[i] - 1;
        size_t map = order % MAP_COUNT;
        size_t stage_b = order / MAP_COUNT;
        stages->maps[map].stages[stage_b] = i + 1;
        stages->maps[map].balls[stage_b] = stages->balls[order];
    }
    return stages;
}

void map_write(map_full* map, FILE* rom) {
    if (map->line_count) {
        map->lines[map->line_count - 1].x |= MAP_END_BIT;
        for (size_t i = 0; i < map->line_count; i++) {
            map_line* line = map->lines + i;
            fwrite(line, ((line->x & TYPE_MASK) == TYPE_BODY) ? 2 : 3, 1, rom);
        }
        map->lines[map->line_count - 1].x ^= MAP_END_BIT;
    }
    if (map->back_count) {
        map->backs[map->back_count - 1].x1 |= MAP_END_BIT;
        fwrite(map->backs, sizeof(map_back), map->back_count, rom);
        map->backs[map->back_count - 1].x1 ^= MAP_END_BIT;
    }
    if (map->hole_count) {
        map->holes[map->hole_count - 1].x |= MAP_END_BIT;
        fwrite(map->holes, sizeof(map_hole), map->hole_count, rom);
        map->holes[map->hole_count - 1].x ^= MAP_END_BIT;
    }
}

void stages_write(lb_stages* stages, FILE* rom) {
    u16 map_table[MAP_COUNT];
    // Write the stage order and the balls
    fseek(rom, ROM_STAGE_ORDER_OFFSET, SEEK_SET);
    fwrite(stages->order, sizeof(stages->order), 1, rom);
    fwrite(stages->balls, sizeof(stages->balls), 1, rom);
    // Write the maps, until we run out of space
    fseek(rom, ROM_MAP_DATA_START, SEEK_SET);
    for (size_t i = 0, bytes = ROM_MAP_DATA_BYTES; i < MAP_COUNT; i++) {
        if (stages->maps[i].byte_count > bytes)
            break;
        bytes -= stages->maps[i].byte_count;
        map_table[i] = ftell(rom) + ROM_RAM_OFFSET;
        map_write(stages->maps + i, rom);
    }
    // Write the resulting map table
    fseek(rom, ROM_MAP_TABLE_OFFSET, SEEK_SET);
    fwrite(map_table, sizeof(map_table), 1, rom);
}

void tile_map_wall(map_tiles* tiles, size_t x, size_t y, u8 wall,
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

void tile_map_line(map_tiles* tiles, const map_line* line, u8* backup,
                     size_t reverse) {
    u8 x = line->x & MAP_POS_MASK;
    u8 y = line->y & MAP_POS_MASK;
    u8 type = line->x & TYPE_MASK;
    if (type == TYPE_BODY) {
        if (line->y & FLAG_BODY_SLANT) {
            static const u8 slope_map[4] = {0x0C, 0x09, 0x06, 0x03};
            for (size_t j = 0; j < 4; j++)
                tile_map_wall(tiles, x + (j & 1), y + (j >> 1),
                                slope_map[j], backup, j, reverse);
        } else if (line->y & FLAG_BODY_BLOCK) {
            tile_map_wall(tiles, x, y, TILE_BLOCK, backup, 0, reverse);
        } else {
            for (size_t j = 0; j < 4; j++)
                tile_map_wall(tiles, x + (j & 1), y + (j >> 1),
                                TILE_BLOCK, backup, j, reverse);
        }
    } else if (type == TYPE_HORIZONTAL) {
        for (size_t j = x; j <= line->end; j++)
            tile_map_wall(tiles, j, y, TILE_BLOCK, backup, j - x, reverse);
    } else if (type == TYPE_VERTICAL) {
        for (size_t j = y; j <= line->end; j++)
            tile_map_wall(tiles, x, j, TILE_BLOCK, backup, j - y, reverse);
    } else {
        static const u8 slope_map[4] = {0x03, 0x06, 0x0C, 0x09};
        u8 tile = slope_map[line->y >> 6];
        if (line->y & FLAG_LINE_BLOCK)
            tile = TILE_BLOCK;
        if (line->end > y)
            for (size_t j = x, k = y; k <= line->end; j++, k++)
                tile_map_wall(tiles, j, k, tile, backup, j - x, reverse);
        else
            for (size_t j = x, k = y; k >= line->end; j++, k--)
                tile_map_wall(tiles, j, k, tile, backup, j - x, reverse);
    }
}

int map_add_line(map_full* map, map_tiles* tiles, size_t x1, size_t y1,
                    size_t x2, size_t y2, size_t tool, u8* backup) {
    if (map->line_count == MAP_MAX_LINES)
        return -1;
    ssize_t x = x1, y = y1, end = y2;
    size_t wall = 0, type = TYPE_DIAGONAL, bytes = sizeof(map_line);
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
    map_line* line = map->lines + map->line_count;
    *line = (map_line) {x | type, y | wall, end};
    tile_map_line(tiles, line, backup, 0);
    map->byte_count += bytes;
    map->line_count++;
    return 0;
}

void tile_map_hole(map_tiles* tiles, map_hole* hole,
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

int map_add_hole(map_full* map, map_tiles* tiles, size_t x, size_t y,
                   u8* backup) {
    if (map->hole_count == MAP_MAX_HOLES)
        return -1;
    map_hole* hole = map->holes + map->hole_count;
    *hole = (map_hole) {x, y};
    tile_map_hole(tiles, hole, backup, 0);
    map->hole_count++;
    map->byte_count += sizeof(map_hole);
    return 0;
}

int map_add_back(map_full* map, map_tiles* tiles,
                   size_t x1, size_t y1, size_t x2, size_t y2) {
    if (map->back_count == MAP_MAX_BACKS)
        return -1;
    size_t tx1 = x1, tx2 = x2, ty1 = y1, ty2 = y2;
    if (x1 > x2)
        tx1 = x2, tx2 = x1;
    if (y1 > y2)
        ty1 = y2, ty2 = y1;
    map->backs[map->back_count] = (map_back) {tx1, ty1, tx2, ty2};
    tiles->backs[tiles->back_count] = (map_back) {tx1, ty1, tx2, ty2};
    tiles->back_count++;
    map->back_count++;
    map->byte_count += sizeof(map_back);
    return 0;
}

void map_remove_line(map_full* map, map_tiles* tiles, u8* backup) {
    map->line_count--;
    map->byte_count -= sizeof(map_line) -
        ((map->lines[map->line_count].x & TYPE_BODY) == TYPE_BODY);
    tile_map_line(tiles, map->lines + map->line_count, backup, 1);
}

void map_remove_hole(map_full* map, map_tiles* tiles, u8* backup) {
    map->hole_count--;
    map->byte_count -= sizeof(map_hole);
    tile_map_hole(tiles, map->holes + map->hole_count, backup, 1);
}

void map_remove_back(map_full* map, map_tiles* tiles) {
    map->back_count--;
    map->byte_count -= sizeof(map_back);
    tiles->back_count--;
}

void map_clear(map_full* map, map_tiles* tiles) {
    uintptr_t start = (uintptr_t) &map->byte_count;
    uintptr_t end = (uintptr_t) (map + 1);
    memset((void*) start, 0, end - start);
    memset(tiles, 0, sizeof(map_tiles));
}

void tile_map_lines(map_tiles* tiles, const map_line* lines,
                      size_t count) {
    for (size_t i = 0; i < count; i++)
        tile_map_line(tiles, lines + i, NULL, 0);
}

void init_map_tiles(map_tiles* tiles, map_full* map) {
    memset(tiles, 0, sizeof(*tiles));
    tile_map_lines(tiles, map->lines, map->line_count);
    for (size_t i = 0; i < map->hole_count; i++)
        tile_map_hole(tiles, map->holes + i, NULL, 0);
    memcpy(tiles->backs, map->backs, sizeof(map->backs));
    tiles->back_count = map->back_count;
}
