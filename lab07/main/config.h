#include "hw_gc.h"

#ifndef CONFIG_H_
#define CONFIG_H_
// hardware
#define SCREEN_WIDTH HW_LCD_W
// Player
#define START_X_POS 30
#define HURTBOX_WIDTH 20
#define HURTBOX_HEIGHT 60
#define WALK_SPEED 5
#define AIR_DRIFT_SPEED 3
//Level
#define GROUND_LEVEL 220
#define SIDE_SCREEN_OFFSET 10
// Time
#define ATTACK_DELAY_MS 500

#define GROUND_COLOR 0x6204  // brown
#define CONFIG_COLOR_BACKGROUND rgb565(0, 4, 16)
#define PLAYER_COLOR 0xFF00  //temporary until we get player sprite

#define JOY_Y_THRES 1000
#define JOY_X_THRES 200

#define CONFIG_GAME_TIMER_PERIOD 40.0E-3f

#define GAME_END 5000

// Attack Durations (in Game Ticks)
// Assuming 1 tick = 40ms (from your CONFIG_GAME_TIMER_PERIOD)
#define PUNCH_DURATION 10       // 10 ticks * 40ms = 400ms total
#define KICK_DURATION 15        // 15 ticks * 40ms = 600ms total
#define ATTACK_COOLDOWN 5       // Wait 5 ticks before attacking again
#define PUNCH_DURATION_FRAMES  10
#define KICK_DURATION_FRAMES   15
#define PUNCH_COOLDOWN_FRAMES 5
#define KICK_COOLDOWN_FRAMES 20

// Attack Hitbox Timing (When does the punch actually hit?)
// The punch starts at frame 0, but maybe the fist is only fully extended
// from frame 3 to frame 6.
#define PUNCH_ACTIVE_START 3
#define PUNCH_ACTIVE_END 6

#define KICK_ACTIVE_START 4
#define KICK_ACTIVE_END 8

#define PUNCH_RANGE_X 25
#define PUNCH_RANGE_Y 10
#define KICK_RANGE_X 30
#define KICK_RANGE_Y 10

// health stuff
#define MAX_HEALTH 500

// Health Bar Dimensions
#define HEALTH_BAR_W 50
#define HEALTH_BAR_H 6
#define HEALTH_BAR_Y 4  // Distance from top of screen

// Positions
#define P1_HEALTH_X 4   // Top Left
#define P2_HEALTH_X (HW_LCD_W - HEALTH_BAR_W - 4) // Top Right

// Colors
#define COLOR_HEALTH_FILLED GREEN
#define COLOR_HEALTH_EMPTY  RED
#define COLOR_HEALTH_BORDER WHITE

#define PUNCH_DAMAGE 20
#define KICK_DAMAGE 30

#define HURT_DURATION 15   // Duration of hit stun/invincibility in frames

#endif