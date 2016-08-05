#pragma once

#include <stage.h>

#define HISTORY_SIZE 256

typedef struct action_tool {
    size_t tool, x1, y1, x2, y2;
    stage_ball* ball;
    u8 backup[GRID_WIDTH];
} action_tool;

typedef struct history {
    size_t index, count, active;
    action_tool history[HISTORY_SIZE];
} history;

history* history_init();
void history_do(history* history, table_full* table, table_tiles* tiles,
                size_t stage_b, size_t tool, size_t x, size_t y,
                size_t* invalid_layers);
void history_undo(history* history, table_full* table, table_tiles* tiles,
                  size_t* invalid_layers);
void history_redo(history* history, table_full* table, table_tiles* tiles,
                  size_t x, size_t y, size_t* invalid_layers);
void history_clear(history* history);
