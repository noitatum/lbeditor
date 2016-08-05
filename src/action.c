#include <string.h>
#include <action.h>
#include <hud.h>
#include <render.h>

void history_apply(history* history, map_full* map, map_tiles* tiles,
                   size_t* invalid_layer) {
    action_tool* action = history->history + history->index;
    if (action->ball) {
        history->active = 1;
        action->ball->x = action->x2 / 2, action->ball->y = action->y2 / 2;
        *invalid_layer |= 1 << LAYER_BALLS;
    } else {
        size_t x1 = action->x1 / TSIZE, y1 = action->y1 / TSIZE;
        size_t x2 = action->x2 / TSIZE, y2 = action->y2 / TSIZE;
        if (action->tool == TOOL_HOLE) {
            history->active = !map_add_hole(map, tiles, x2, y2);
            *invalid_layer |= 1 << LAYER_HOLES;
        } else if (action->tool == TOOL_BACK) {
            history->active = !map_add_back(map, x1, y1, x2, y2);
            *invalid_layer |= 1 << LAYER_BACK;
        } else if (IS_TOOL_LINE(action->tool)) {
            history->active = !map_add_line(map, tiles, x1, y1, x2, y2,
                                            action->tool);
            *invalid_layer |= 1 << LAYER_WALLS;
        }
    }
    // If the action didn't fail
    if (history->active) {
        if (history->count < HISTORY_SIZE)
            history->count++;
        if (history->index < HISTORY_SIZE - 1)
            history->index++;
        else
            history->index = 0;
    }
}

void history_do(history* history, map_full* map, map_tiles* tiles,
                size_t stage_b, size_t tool, size_t x, size_t y,
                size_t* invalid_layer) {
    action_tool* action = history->history + history->index;
    action->x1 = action->x2 = x, action->y1 = action->y2 = y;
    action->tool = tool, action->ball = NULL;
    if (IS_TOOL_BALL(tool)) {
        action->ball = map->balls[stage_b] + tool - TOOL_BALL_0;
        action->x1 = action->ball->x, action->y1 = action->ball->y;
    }
    history_apply(history, map, tiles, invalid_layer);
    // If something changed we need to redraw the hud
    if (history->active)
        *invalid_layer |= 1 << LAYER_HUD;
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
        *invalid_layer |= 1 << LAYER_HOLES;
        map_remove_hole(map, tiles, map->hole_count - 1);
    } else if (action->tool == TOOL_BACK) {
        map_remove_back(map, map->back_count - 1);
        *invalid_layer |= 1 << LAYER_BACK;
    } else if (IS_TOOL_LINE(action->tool)) {
        map_remove_line(map, tiles, map->line_count - 1);
        *invalid_layer |= 1 << LAYER_WALLS;
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
