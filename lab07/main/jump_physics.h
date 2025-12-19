#ifndef JUMP_PHYSICS_H_
#define JUMP_PHYSICS_H_

#include <stdint.h> // For uint8_t
#include <stdbool.h> // For bool

// This table has 41 elements (frames 0 through 40).
#define JUMP_TABLE_SIZE 41

/**
 * @brief Pre-calculated jump displacement table.
 * Each value is the number of pixels *above* PLAYER_GROUND_Y.
 * This was generated with:
 * - Initial Velocity: 10.0 pixels/frame
 * - Gravity: 0.5 pixels/frame^2
 * This results in a 100-pixel high jump over 40 frames.
 */
const uint8_t jump_table[JUMP_TABLE_SIZE] = {
    0, 10, 19, 28, 36, 44, 51, 58, 64, 70, 75, 
    80, 84, 88, 91, 94, 96, 98, 99, 100, 100, 
    100, 99, 98, 96, 94, 91, 88, 84, 80, 75, 
    70, 64, 58, 51, 44, 36, 28, 19, 10, 0, 
};

#endif