#include "ent_movable.h"

ent_movable::ent_movable(game_world &world, entity_type type) : entity(world, type) {}

bool ent_movable::std_move(float dx, float dy, uint8_t flags) {
    do_send_updates = true;

    float to_fx = fx;
    float to_fy = fy;

    float check_fx = fx;
    float check_fy = fy;

    float move_speed = 0;
    uint8_t dir;
    bool moved = false;

    if (dy < 0) {
        dir = 0;
        move_speed = -dy;
        to_fy += dy;
        check_fy = to_fy - TILE_SIZE / 2;
    } else if (dy > 0) {
        dir = 1;
        move_speed = dy;
        to_fy += dy;
        check_fy = to_fy + TILE_SIZE / 2;
    } else if (dx < 0) {
        dir = 2;
        move_speed = -dx;
        to_fx += dx;
        check_fx = to_fx - TILE_SIZE / 2;
    } else if (dx > 0) {
        dir = 3;
        move_speed = dx;
        to_fx += dx;
        check_fx = to_fx + TILE_SIZE / 2;
    } else {
        return false;
    }

    int to_tx = check_fx / TILE_SIZE + 0.5f;
    int to_ty = check_fy / TILE_SIZE + 0.5f;

    if ((to_tx == getTileX() && to_ty == getTileY() && !world.isWalkable(fx, fy, flags)) ||
        world.isWalkable(check_fx, check_fy, flags)) {

        fx = to_fx;
        fy = to_fy;
        moved = true;
    }

    switch (dir) {
    case 0:
    case 1:
    {
        float side_dx = getTileX() * TILE_SIZE - fx;
        if (side_dx > 0) fx += side_dx > move_speed ? move_speed : side_dx;
        else if (side_dx < 0) fx += side_dx < -move_speed ? -move_speed : side_dx;
        break;
    }
    case 2:
    case 3:
    {
        float side_dy = getTileY() * TILE_SIZE - fy;
        if (side_dy > 0) fy += side_dy > move_speed ? move_speed : side_dy;
        else if (side_dy < 0) fy += side_dy < -move_speed ? -move_speed : side_dy;
        break;
    }
    }

    return moved;
}
