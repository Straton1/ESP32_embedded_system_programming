
#ifndef ATTACK_H_
#define ATTACK_H_

#include <stdbool.h>
#include "game.h"

typedef enum {
    ATTACK_NONE,
    ATTACK_PUNCH,
    ATTACK_KICK
    // Add others like ATTACK_HADOUKEN later
} attack_state_t;

void attack_init(player_t *player);
void attack_tick(player_t *player);

// Helper to read hardware button (placeholder)
bool read_button_A(void); 
bool read_button_B(void);

#endif