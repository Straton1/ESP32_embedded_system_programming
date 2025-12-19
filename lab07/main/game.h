#ifndef GAME_H_
#define GAME_H_

#include "lcd.h"
#include <stdbool.h>

// Player facing direction
typedef enum {
    FACING_RIGHT = 1,
    FACING_LEFT = -1
} direction_e;

// This struct contains all of the information about the player
typedef struct {

	// movement state defined in movement.h
	int32_t movement_state;

	// position coordinates
	coord_t x_loc;
	coord_t y_loc;

	// is the player jumping?
	bool jumping;
	int32_t jump_frame; // tracks the position of the jump 

	// is player ducking?
	bool ducking;

	// player health
	int16_t health;

	// player facing direction
	direction_e facing;

	// --- ATTACK FIELDS ---
    int32_t attack_state; // Which attack is happening
    int32_t attack_frame;        // Current frame of the attack animation
    bool is_attacking;           // Flag to lock movement while attacking
    int32_t cooldown_timer;      // Frames before player can attack again

	// --- VISUAL FLAGS ---
    bool is_bad_guy;            // True if this is the remote player
    int32_t hurt_timer;         // Frames to display "hurt" sprite

	bool ready_to_start;  // True if player pressed START on title screen
    bool wants_reset;     // True if player pressed RESET button during game
    bool accept_reset;    // True if player accepted the opponent's reset request

}player_t;

typedef struct {
    int16_t x_loc;       // Where are they?
    int16_t y_loc;
    uint8_t facing;      // Left or Right?
    bool is_attacking;   // Are they hitting?
    uint8_t attack_state;// Punch or Kick?
    bool ducking;        // Are they ducking?
	int16_t health;

	bool ready_to_start;
    bool wants_reset;
    bool accept_reset;
} player_net_state_t;

void game_tick(void);

void game_init(void);

bool player_is_moving(player_t *player);

int32_t get_player_height(player_t *player);

void game_populate_bad_guy(player_t *remote_player, player_net_state_t *incoming_packet);

void game_populate_packet(player_t *player, player_net_state_t *packet);

bool game_hit_registered(player_t *good, player_t *bad);

void draw_health_bar(coord_t x, coord_t y, int16_t current, int16_t max);

// void game_over(int16_t good_health, int16_t bad_health);

// a function to handle resetting variables
void game_reset_round(void);

#endif