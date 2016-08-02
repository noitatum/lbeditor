#include <string.h>
#include <action.h>
#include <hud.h>

void action_history_apply(action_history* history, table_full* table,
                          table_tiles* tiles) {
    action_tool* action = history->history + history->index;
    if (action->tool == TOOL_HOLE)
        history->active = !table_add_hole(table, tiles, action->x2, action->y2, 
                                          action->backup);
    else if (action->tool == TOOL_BACK)
        history->active = !table_add_back(table, tiles, action->x1, action->y1,
                                         action->x2, action->y2);
    else if (action->tool <= TOOL_BACK)
        history->active = !table_add_line(table, tiles, action->x1, action->y1,
                                          action->x2, action->y2, action->tool, 
                                          action->backup);
    if (history->count < HISTORY_SIZE)
        history->count++;
    if (history->index < HISTORY_SIZE - 1)
        history->index++;
    else
        history->index = 0;
}

void action_history_do(action_history* history, table_full* table,
                       table_tiles* tiles, size_t  tool, size_t x, size_t y) {
    action_tool* action = history->history + history->index;
    action->x1 = action->x2 = x, action->y1 = action->y2 = y;
    action->tool = tool;
    action_history_apply(history, table, tiles);
}

void action_history_undo(action_history* history, table_full* table,
                         table_tiles* tiles) {
    if (history->count == 0) 
        return;
    history->count--;
    if (history->index > 0)
        history->index--;
    else
        history->index = HISTORY_SIZE - 1;
    action_tool* action = history->history + history->index;
    if (action->tool == TOOL_HOLE)
        table_remove_hole(table, tiles, action->backup);
    else if (action->tool == TOOL_BACK)
        table_remove_back(table, tiles);
    else if (action->tool <= TOOL_BACK)
        table_remove_line(table, tiles, action->backup); 
}

void action_history_redo(action_history* history, table_full* table,
                         table_tiles* tiles, size_t x, size_t y) {
    if (history->count == 0)
        return;
    size_t index = HISTORY_SIZE - 1;
    if (history->index > 0)
        index = history->index - 1;
    action_tool* action = history->history + index;
    if (x != action->x2 || y != action->y2) {
        action_history_undo(history, table, tiles);
        action->x2 = x, action->y2 = y;
        action_history_apply(history, table, tiles);
    }
}

void action_history_clear(action_history* history) {
    memset(history, 0, sizeof(action_history));
}

action_history* action_history_init() {
    action_history* history = malloc(sizeof(action_history));
    memset(history, 0, sizeof(action_history));
    return history;
}
