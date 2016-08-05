#pragma once

#include <stage.h>

#define HISTORY_SIZE 256

typedef struct action_tool {
    size_t tool, x1, y1, x2, y2;
    stage_ball* ball;
} action_tool;

typedef struct history {
    size_t index, count, active;
    action_tool history[HISTORY_SIZE];
} history;

typedef struct history_table {
    history table[MAP_COUNT];
} history_table;

history_table* history_init();
void history_do(history* history, map_full* map, map_tiles* tiles,
                size_t stage_b, size_t tool, size_t x, size_t y,
                size_t* invalid_layers);
void history_undo(history* history, map_full* map, map_tiles* tiles,
                  size_t* invalid_layers);
void history_redo(history* history, map_full* map, map_tiles* tiles,
                  size_t x, size_t y, size_t* invalid_layers);
void history_clear(history* history);
