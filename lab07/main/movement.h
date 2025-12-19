#ifndef MOVEMENT_H_
#define MOVEMENT_H_

#include <stdint.h>
#include <stdbool.h>
#include "game.h"

// movement states
typedef enum {
    MOVEMENT_STATIONARY,
    MOVEMENT_LEFT,
    MOVEMENT_RIGHT,
    MOVEMENT_JUMP,
    MOVEMENT_DUCK
}movement_state_t;

// initialize functions

//
void movement_init(player_t *player);

//
void movement_tick(player_t *player, player_t *enemy);

int32_t read_joystick_y(void);

int32_t read_joystick_x(void);

bool movement_is_enemy_collision_right(coord_t player_x, coord_t enemy_x);
bool movement_is_enemy_collision_left(coord_t player_x, coord_t enemy_x);

#endif