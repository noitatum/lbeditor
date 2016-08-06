#include <string.h>
#include <action.h>
#include <hud.h>
#include <render.h>

void history_apply(history* history, map_full* map, map_tiles* tiles,
                   size_t* invalid_layer) {
    action_tool* action = history->history + history->index;
    size_t layer = 0;
    int index = 0;
    if (action->ball) {
        if (action->remove)
            action->ball->x = action->ball->y = 0;
        else
            action->ball->x = action->x2 / 2, action->ball->y = action->y2 / 2;
        layer = 1 << LAYER_BALLS;
    } else {
        size_t x1 = action->x1 / TSIZE, y1 = action->y1 / TSIZE;
        size_t x2 = action->x2 / TSIZE, y2 = action->y2 / TSIZE;
        size_t tool = action->tool;
        if (tool == TOOL_HOLE) {
            if (action->remove) {
                index = map_find_hole(map, x2, y2);
                if (index != -1) {
                    action->hole = map->holes[index];
                    map_remove_hole(map, tiles, index);
                }
            } else {
                index = map_add_hole_action(map, tiles, x2, y2);
            }
            layer =  1 << LAYER_HOLES;
        } else if (tool == TOOL_BACK) {
            if (action->remove) {
                index = map_find_back(map, x2, y2);
                if (index != -1) {
                    action->back = map->backs[index];
                    map_remove_back(map, index);
                }
            } else {
                index = map_add_back_action(map, x1, y1, x2, y2);
            }
            layer = 1 << LAYER_BACK;
        } else if (IS_TOOL_LINE(tool)) {
            if (action->remove) {
                index = map_find_line(map, x2, y2);
                if (index != -1) {
                    action->line = map->lines[index];
                    map_remove_line(map, tiles, index);
                }
            } else {
                index = map_add_line_action(map, tiles, x1, y1, x2, y2, tool);
            }
            layer = 1 << LAYER_WALLS;
        }
    }
    action->index = index;
    if (index != -1)
        *invalid_layer |= layer | (1 << LAYER_HUD);
    if (history->count < HISTORY_SIZE)
        history->count++;
    if (history->index < HISTORY_SIZE - 1)
        history->index++;
    else
        history->index = 0;
}

void history_do(history* history, map_full* map, map_tiles* tiles,
                size_t stage_b, size_t tool, size_t x, size_t y,
                size_t remove, size_t* invalid_layer) {
    action_tool* action = history->history + history->index;
    action->x1 = action->x2 = x, action->y1 = action->y2 = y;
    action->tool = tool, action->ball = NULL, action->remove = remove;;
    if (IS_TOOL_BALL(tool)) {
        action->ball = map->balls[stage_b] + tool - TOOL_BALL_0;
        action->x1 = action->ball->x, action->y1 = action->ball->y;
    }
    history_apply(history, map, tiles, invalid_layer);
}

void history_undo(history* history, map_full* map, map_tiles* tiles,
                  size_t* invalid_layer) {
    if (history->count == 0)
        return;
    history->count--;
    if (history->index > 0)
        history->index--;
    else
        history->index = HISTORY_SIZE - 1;
    action_tool* action = history->history + history->index;
    if (action->tool == TOOL_HOLE) {
        if (action->index != -1) {
            if (action->remove)
                map_add_hole(map, tiles, &action->hole, action->index);
            else
                map_remove_hole(map, tiles, map->hole_count - 1);
            *invalid_layer |= 1 << LAYER_HOLES;
        }
    } else if (action->tool == TOOL_BACK) {
        if (action->index != -1) {
            if (action->remove)
                map_add_back(map, &action->back, action->index);
            else
                map_remove_back(map, map->back_count - 1);
            *invalid_layer |= 1 << LAYER_BACK;
        }
    } else if (IS_TOOL_LINE(action->tool)) {
        if (action->index != -1) {
            if (action->remove)
                map_add_line(map, tiles, &action->line, action->index);
            else
                map_remove_line(map, tiles, map->line_count - 1);
            *invalid_layer |= 1 << LAYER_WALLS;
        }
    } else if (action->ball) {
        action->ball->x = action->x1;
        action->ball->y = action->y1;
        *invalid_layer |= 1 << LAYER_BALLS;
    }
    // Redraw the hud
    *invalid_layer |= 1 << LAYER_HUD;
}

void history_redo(history* history, map_full* map, map_tiles* tiles,
                  size_t x, size_t y, size_t* invalid_layer) {
    if (history->count == 0)
        return;
    history_undo(history, map, tiles, invalid_layer);
    action_tool* action = history->history + history->index;
    action->x2 = x, action->y2 = y;
    history_apply(history, map, tiles, invalid_layer);
}

void history_clear(history* history) {
    memset(history, 0, sizeof(*history));
}

history_table* history_init() {
    history_table* history = malloc(sizeof(history_table));
    memset(history, 0, sizeof(history_table));
    return history;
}
