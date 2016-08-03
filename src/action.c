#include <string.h>
#include <action.h>
#include <hud.h>
#include <render.h>

void history_apply(history* history, table_full* table,
                   table_tiles* tiles, stage_ball* balls, size_t* invalid_layer) {
    action_tool* action = history->history + history->index;
    if (IS_TOOL_BALL(action->tool)) {
        history->active = 1;
        stage_ball* ball = balls + action->tool - TOOL_BALL_0;
        ball->x = action->x2 / 2, ball->y = action->y2 / 2;
        *invalid_layer |= 1 << LAYER_BALLS;
    } else {
        size_t x1 = action->x1 / TSIZE, y1 = action->y1 / TSIZE;
        size_t x2 = action->x2 / TSIZE, y2 = action->y2 / TSIZE;
        u8* backup = action->backup;
        if (action->tool == TOOL_HOLE) {
            history->active = !table_add_hole(table, tiles, x2, y2, backup);
            *invalid_layer |= 1 << LAYER_HOLES;
        } else if (action->tool == TOOL_BACK) {
            history->active = !table_add_back(table, tiles, x1, y1, x2, y2);
            *invalid_layer |= 1 << LAYER_BACK;
        } else if (IS_TOOL_LINE(action->tool)) {
            history->active = !table_add_line(table, tiles, x1, y1, x2, y2,
                                              action->tool, backup);
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

void history_do(history* history, table_full* table, table_tiles* tiles,
                stage_ball* balls, size_t tool, size_t x, size_t y,
                size_t* invalid_layer) {
    action_tool* action = history->history + history->index;
    action->x1 = action->x2 = x, action->y1 = action->y2 = y;
    action->tool = tool;
    if (IS_TOOL_BALL(action->tool)) {
        stage_ball* ball = balls + action->tool - TOOL_BALL_0;
        action->backup[0] = ball->x, action->backup[1] = ball->y;
    }
    history_apply(history, table, tiles, balls, invalid_layer);
    // If something changed we need to redraw the hud
    if (history->active)
        *invalid_layer |= 1 << LAYER_HUD;
}

void history_undo(history* history, table_full* table, table_tiles* tiles,
                  stage_ball* balls, size_t* invalid_layer) {
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
        table_remove_hole(table, tiles, action->backup);
    } else if (action->tool == TOOL_BACK) {
        table_remove_back(table, tiles);
        *invalid_layer |= 1 << LAYER_BACK;
    } else if (IS_TOOL_LINE(action->tool)) {
        table_remove_line(table, tiles, action->backup);
        *invalid_layer |= 1 << LAYER_WALLS;
    } else if (IS_TOOL_BALL(action->tool)) {
        stage_ball* ball = balls + action->tool - TOOL_BALL_0;
        ball->x = action->backup[0], ball->y = action->backup[1];
        *invalid_layer |= 1 << LAYER_BALLS;
    }
    // Redraw the hud
    *invalid_layer |= 1 << LAYER_HUD;
}

void history_redo(history* history, table_full* table, table_tiles* tiles,
                  stage_ball* balls, size_t x, size_t y,
                  size_t* invalid_layer) {
    if (history->count == 0)
        return;
    history_undo(history, table, tiles, balls, invalid_layer);
    action_tool* action = history->history + history->index;
    action->x2 = x, action->y2 = y;
    history_apply(history, table, tiles, balls, invalid_layer);
}

void history_clear(history* history) {
    memset(history, 0, sizeof(*history));
}

history* history_init() {
    history* history = malloc(sizeof(*history));
    memset(history, 0, sizeof(*history));
    return history;
}
