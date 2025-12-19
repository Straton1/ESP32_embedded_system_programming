#include "movement.h"
#include <stdlib.h>
#include "lcd.h"
#include "config.h"
#include "game.h"
#include "pin.h"
#include "hw.h"
#include "jump_physics.h"
#include "joy.h"
#include "esp_log.h"

// static const char* TAG = "MyModule"; 

// defined values and macros

// global variables
static int32_t drift_direction;
static int16_t stationary_frames = 10;

// Gloabal helper functions

int32_t read_joystick_x(void)
{
    int32_t dcx;
    int32_t dcy;
    joy_get_displacement(&dcx, &dcy);
    // ESP_LOGE(TAG, "dcx = %ld", dcx);
    if (dcx > JOY_X_THRES)
    {
        return 1;
    }
    else if (dcx < -1*JOY_X_THRES)
    {
        return -1;
    }
    return 0;
}

int32_t read_joystick_y(void)
{
    int32_t dcx;
    int32_t dcy;
    joy_get_displacement(&dcx, &dcy);
    // ESP_LOGE(TAG, "dcy = %ld", dcy);
    if (dcy > JOY_Y_THRES)
    {
        return -1;
    }
    else if (dcy < -1*JOY_Y_THRES)
    {
        return 1;
    }
    return 0;
}

bool movement_is_enemy_collision_right(coord_t player_x, coord_t enemy_x)
{
    // player is on the left of the enemy
    if (player_x < enemy_x)
    {
        // player is touching the enemy and can't move any closer
        if ((enemy_x - player_x) <= HURTBOX_WIDTH)
        {
            return true;
        }
    }
    
    // default return false
    return false;
}

bool movement_is_enemy_collision_left(coord_t player_x, coord_t enemy_x)
{
    // player is on the right of the enemy
    if (player_x > enemy_x)
    {
        // player is touching the enemy and can't move any closer
        if ((player_x - enemy_x) <= HURTBOX_WIDTH)
        {
            return true;
        }
    }
    
    // default return false
    return false;
}

/************** Movement Init Functions (and helpers) *****************/

// initialize the movement variables within the player struct
void movement_init(player_t *player)
{
    // Constrain player to screen boundaries
    if (player->x_loc < SIDE_SCREEN_OFFSET) //
    {
        player->x_loc = SIDE_SCREEN_OFFSET;
    }
    
    // Calculate the right-side boundary
    int right_boundary = SCREEN_WIDTH - SIDE_SCREEN_OFFSET - HURTBOX_WIDTH; //
    
    if (player->x_loc > right_boundary)
    {
        player->x_loc = right_boundary;
    }
    player->movement_state = MOVEMENT_STATIONARY;
    player->x_loc = START_X_POS;
    player->y_loc = GROUND_LEVEL;
}

/************** Movement Tick Function (and helpers) *****************/

void movement_tick(player_t *player, player_t *enemy)
{
    // --- Read Inputs Once ---
    // (These are fictional functions, replace with your actual hardware calls)
    int32_t joystick_x = read_joystick_x(); // e.g., returns -1, 0, or 1
    int32_t joystick_y = read_joystick_y(); // e.g., returns -1, 0, or 1
    
    // Disable controls if hurt (Immobilization), but allow gravity to continue
    if (player->hurt_timer > 0)
    {
        joystick_x = 0;
        joystick_y = 0;
    }

    // =======================================================
    // Part 1: Handle Vertical (Jumping) Logic
    // This logic runs *in parallel* to the horizontal FSM.
    // =======================================================

    // --- Check for a NEW jump ---
    // Can only start a jump if NOT already jumping and joystick is UP
    if (joystick_y == 1 /*UP*/ && player->jumping == false) 
    {
        player->jumping = true;
        player->jump_frame = 0;
        // set in air drift
        if (joystick_x == -1 /*LEFT*/) {
            drift_direction = -1*AIR_DRIFT_SPEED;
        }
        else if (joystick_x == 1 /*RIGHT*/) {
            drift_direction = AIR_DRIFT_SPEED;
        }
        else
        {
            drift_direction = 0;
        }
    }

    // --- Update an ONGOING jump ---
    if (player->jumping == true) 
    {
        // Increment the frame in the jump animation
        player->jump_frame++;

        // Check if the jump is over (reached end of table)
        if (player->jump_frame >= JUMP_TABLE_SIZE) //
        {
            player->jumping = false;
            player->y_loc = GROUND_LEVEL; //
        } 
        else 
        {
            // Get the vertical offset from our pre-calculated table
            uint8_t jump_offset = jump_table[player->jump_frame]; //
            
            // Update Y position. (Y=0 is top of screen, so we subtract)
            player->y_loc = GROUND_LEVEL - jump_offset; //
        }

        // --- Handle optional in-air drift ---
        // We can move left/right in the air without changing the horizontal *state*
        player->x_loc += drift_direction;
    }


    // =======================================================
    // Part 2: Handle Horizontal (Grounded) FSM Transitions
    // This logic only runs if we are ON THE GROUND.
    // =======================================================


    if (player->jumping == false) 
    {
        // --- Transition Logic ---
        switch (player->movement_state)
        {
            case MOVEMENT_STATIONARY:
                // stay stationary for a certain number of frames
                stationary_frames--;
                if (stationary_frames < 0)
                {
                    // make sure there is no attack going on
                    if (!player->is_attacking)
                    {
                        if (joystick_y == -1 /*DOWN*/) {
                            player->movement_state = MOVEMENT_DUCK;
                        }
                        else if ((joystick_x == -1) && !movement_is_enemy_collision_left(player->x_loc, enemy->x_loc) /*LEFT*/) {
                            player->movement_state = MOVEMENT_LEFT;
                        }
                        else if (joystick_x == 1 && !movement_is_enemy_collision_right(player->x_loc, enemy->x_loc) /*RIGHT*/) {
                            player->movement_state = MOVEMENT_RIGHT;
                        }
                    }
                }
                break;

            case MOVEMENT_LEFT:
                // mealy action, set stationary frame countdown
                stationary_frames = 2;
                // is there a collision with the enemy on the left (the direction we are moving)
                if (movement_is_enemy_collision_left(player->x_loc, enemy->x_loc))
                {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                // if the player is attcking, go stationary
                if (player->is_attacking)
                {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                // if ducking, go to ducking
                else if (joystick_y == -1 /*DOWN*/) {
                        player->movement_state = MOVEMENT_DUCK;
                    }
                // If joystick is released, go back to stationary
                else if (joystick_x != -1 /*LEFT*/) {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                break;

            case MOVEMENT_RIGHT:
                // mealy action, set stationary frame countdown
                stationary_frames = 2;
                // is there a collision with the enemy on the right (the direction we are moving)
                if (movement_is_enemy_collision_right(player->x_loc, enemy->x_loc))
                {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                // if the player is attcking, go stationary
                if (player->is_attacking)
                {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                // if ducking, go to ducking
                else if (joystick_y == -1 /*DOWN*/) {
                        player->movement_state = MOVEMENT_DUCK;
                    }
                // If joystick is released, go back to stationary
                else if (joystick_x != 1 /*RIGHT*/) {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                break;

            case MOVEMENT_DUCK:
                // mealy action, set stationary frame countdown
                stationary_frames = 2;
                // If joystick is released, go back to stationary
                if (joystick_y != -1 /*DOWN*/) {
                    player->movement_state = MOVEMENT_STATIONARY;
                }
                break;

            default:
                player->movement_state = MOVEMENT_STATIONARY;
                break;
        }

        // --- Action Logic ---
        // (This runs *after* the state has been set for this frame)
        switch (player->movement_state)
        {
            case MOVEMENT_STATIONARY:
                // No change in x_loc
                // not ducking
                player->ducking = false;
                break;

            case MOVEMENT_LEFT:
                player->x_loc -= WALK_SPEED;
                // (Set sprite to walk_left)
                break;

            case MOVEMENT_RIGHT:
                player->x_loc += WALK_SPEED;
                // (Set sprite to walk_right)
                break;

            case MOVEMENT_DUCK:
                // No change in x_loc
                // (Set sprite to duck)
                player->ducking = true;
                break;

            default:
                break;
        }
    }
    
    // =======================================================
    // Part 3: Final Cleanup & Constraints
    // This runs every frame, regardless of state.
    // =======================================================

    // Constrain player to screen boundaries
    if (player->x_loc < SIDE_SCREEN_OFFSET) //
    {
        player->x_loc = SIDE_SCREEN_OFFSET;
    }
    
    // Calculate the right-side boundary
    int right_boundary = SCREEN_WIDTH - SIDE_SCREEN_OFFSET - HURTBOX_WIDTH; //
    
    if (player->x_loc > right_boundary)
    {
        player->x_loc = right_boundary;
    }
}

/************** Movement Status Functions *****************/


