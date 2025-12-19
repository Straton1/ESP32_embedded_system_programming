// In lab7/lab07/main/attack.c
#include "esp_log.h"
#include <stdlib.h>
#include "hw_gc.h"
#include "pin.h"
#include "attack.h"
#include "game.h"
#include "movement.h" // Need to know if we are ducking/jumping
#include "config.h"


// static const char* TAG = "AttackModule";

// --- Hardware Helper Placeholders ---
// You will need to replace these with actual calls to your button component
bool read_button_A(void) {
    // return gpio_get_level(BUTTON_A_PIN) == 0; 
    if (!pin_get_level(HW_BTN_A))
    {
        return true;
    }
    return false;
}

bool read_button_B(void) {
    // return gpio_get_level(BUTTON_B_PIN) == 0;
        if (!pin_get_level(HW_BTN_B))
    {
        return true;
    }
    else return false;
}

/************** Attack Init *****************/

void attack_init(player_t *player)
{
    player->attack_state = ATTACK_NONE;
    player->is_attacking = false;
    player->attack_frame = 0;
    player->cooldown_timer = 0;
}

/************** Attack Tick *****************/

void attack_tick(player_t *player)
{
    // 1. Handle Cooldowns
    if (player->cooldown_timer > 0) 
    {
        player->cooldown_timer--;
    }

    bool btn_punch = read_button_A();
    bool btn_kick = read_button_B();

    // =======================================================
    // Part 1: Transition Logic (Start an Attack)
    // =======================================================
    
    // Can only attack if:
    // 1. Not currently attacking
    // 2. Cooldown is 0
    // 3. (Optional) Not hurt/stunned
    switch (player->attack_state)
        {
            case ATTACK_NONE:
                // if cooldown timer is 0, an attack input is acceptable
                if (player->cooldown_timer == 0 && !player->is_attacking)
                {
                    // if a button is pressed, transition to that state
                    if (btn_punch)
                    {
                        player->is_attacking = true; // mealy action to set attacking flag
                        player->attack_state = ATTACK_PUNCH;
                        player->attack_frame = 0; // mealy action to start frame count
                    }
                    else if (btn_kick)
                    {
                        player->is_attacking = true;
                        player->attack_state = ATTACK_KICK;
                        player->attack_frame = 0;
                    }
                }
                break;
                
            case ATTACK_PUNCH:
                // Logic for Punch
                // e.g., Create hitbox at specific frames (frame 3 to 5)
                if (player->attack_frame >= PUNCH_DURATION_FRAMES) {
                    player->is_attacking = false;
                    player->attack_state = ATTACK_NONE;
                    player->cooldown_timer = PUNCH_COOLDOWN_FRAMES;
                }
                break;

            case ATTACK_KICK:
                // Logic for Kick
                if (player->attack_frame >= KICK_DURATION_FRAMES) {
                    player->is_attacking = false;
                    player->attack_state = ATTACK_NONE;
                    player->cooldown_timer = KICK_COOLDOWN_FRAMES;
                }
                break;

            default:
                // Error state recovery
                player->is_attacking = false;
                player->attack_state = ATTACK_NONE;
                break;
        }
    

    // =======================================================
    // Part 2: Action Logic (Execute Attack Animation)
    // =======================================================

    if (player->is_attacking)
    {
        player->attack_frame++;

        switch (player->attack_state)
        {
            case ATTACK_PUNCH:
                // Logic for Punch
                // e.g., Create hitbox at specific frames (frame 3 to 5)
                break;

            case ATTACK_KICK:
                // Logic for Kick
                break;

            default:
                // Error state recovery
                break;
        
        }
    }
}