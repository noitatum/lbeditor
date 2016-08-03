#pragma once

#include <stage.h>

#define HISTORY_SIZE 256

typedef struct action_tool {
    size_t tool, x1, y1, x2, y2;
    u8 backup[GRID_WIDTH];
} action_tool;

typedef struct action_history {
    size_t index, count, active;
    action_tool history[HISTORY_SIZE];
} action_history;

action_history* action_history_init();
void action_history_do(action_history* history, table_full* table,
                       table_tiles* tiles, size_t tool, size_t x, size_t y,
                       size_t* invalid_layers);
void action_history_undo(action_history* history, table_full* table,
                         table_tiles* tiles, size_t* invalid_layers);
void action_history_redo(action_history* history, table_full* table,
                         table_tiles* tiles, size_t x, size_t y,
                         size_t* invalid_layers);
void action_history_clear(action_history* history);
