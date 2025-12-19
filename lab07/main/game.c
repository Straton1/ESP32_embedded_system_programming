// Game Function

#include "game.h"
#include "attack.h"
#include "lcd.h"
#include "movement.h"
#include "config.h"
#include "sprite_renderer.h"
#include "uart.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

// declare tag for debugging
// static const char *TAG = "lab07";

// gloabal variables

// initialize the good guy
player_t good_guy;
player_t bad_guy;
static player_net_state_t myPacket;
static player_net_state_t yourPacket;

// Track previous position for clearing
static coord_t good_prev_x = 0;
static coord_t bad_prev_x = 0;
static coord_t good_prev_y = 0;
static coord_t bad_prev_y = 0;

void game_populate_bad_guy(player_t *remote_player, player_net_state_t *incoming_packet)
{
    remote_player->x_loc = incoming_packet->x_loc;
    remote_player->y_loc = incoming_packet->y_loc;
    remote_player->facing = incoming_packet->facing;
    remote_player->is_attacking = incoming_packet->is_attacking;
    remote_player->attack_state = incoming_packet->attack_state;
    remote_player->ducking = incoming_packet->ducking;
    remote_player->health = incoming_packet->health;

    remote_player->ready_to_start = incoming_packet->ready_to_start;
    remote_player->wants_reset    = incoming_packet->wants_reset;
    remote_player->accept_reset   = incoming_packet->accept_reset;
}

void game_populate_packet(player_t *player, player_net_state_t *packet)
{
    packet->x_loc = SCREEN_WIDTH - (HURTBOX_WIDTH + player->x_loc) ;
    packet->y_loc = player->y_loc;
    packet->facing = player->facing;
    packet->is_attacking = player->is_attacking;
    packet->attack_state = player->attack_state;
    packet->ducking = player->ducking;
    packet->health = player->health;

    packet->ready_to_start = player->ready_to_start;
    packet->wants_reset    = player->wants_reset;
    packet->accept_reset   = player->accept_reset;
}

// function to reset the round
void game_reset_round(void)
{
    // Reset positions
    good_guy.x_loc = START_X_POS;
    good_guy.y_loc = GROUND_LEVEL;
    // Reset health
    good_guy.health = MAX_HEALTH;
    good_guy.hurt_timer = 0;
    
    // Reset State Flags
    good_guy.movement_state = MOVEMENT_STATIONARY;
    good_guy.ready_to_start = false;
    good_guy.wants_reset = false;
    good_guy.accept_reset = false;

    // Reset Enemy (visually)
    bad_guy.health = MAX_HEALTH;
    bad_guy.x_loc = START_X_POS; // Temporarily put them here until packet updates
    bad_guy.ready_to_start = false;
    bad_guy.wants_reset = false;
    bad_guy.accept_reset = false;
}

bool game_hit_registered(player_t *good, player_t *bad)
{
    // Prevent infinite damage/stun lock
    if (good->hurt_timer > 0)
    {
        return false;
    }

    // bad guy is not attacking
    if (!bad->is_attacking)
    {
        return false;
    }

    // bad guy is punching
    if (bad->attack_state == ATTACK_PUNCH)
    {
        // ... (check ranges) ...
        if (abs(good->x_loc - bad->x_loc) <= PUNCH_RANGE_X)
        {
            if (abs(good->y_loc - bad->y_loc) <= PUNCH_RANGE_Y)
            {
                if (!good->ducking) { // Only hit if not ducking
                    good->health -= PUNCH_DAMAGE;
                    good->hurt_timer = HURT_DURATION; // Show hurt animation for 15 frames
                    return true;
                }
            }
        }
    }

    // bad guy is kicking
    if (bad->attack_state == ATTACK_KICK)
    {
        // ... (check ranges) ...
        if (abs(good->x_loc - bad->x_loc) <= KICK_RANGE_X)
        {
            if (abs(good->y_loc - bad->y_loc) <= KICK_RANGE_Y)
            {
                if (!good->ducking) { // Only hit if not ducking
                    good->health -= KICK_DAMAGE;
                    good->hurt_timer = HURT_DURATION; // Show hurt animation for 15 frames
                    return true;
                }
            }
        }
    }

    return false;
}


// void game_over(int16_t good_health, int16_t bad_health)
// {
//     if (good_health <= 0)
//     {
//         // print you lose screen
//         lcd_fillScreen(RED);
//         lcd_setFontSize(2); // Make text bigger
//         lcd_drawString(10, 50, "YOU LOSE!", WHITE);
//         lcd_writeFrame();
//         while (true)
//         {
//             vTaskDelay(GAME_END);
//         }
//     }
//     if (bad_health <= 0)
//     {
//         // print you win screen
//         lcd_fillScreen(GREEN);
//         lcd_setFontSize(2); // Make text bigger (2x normal size)
//         lcd_drawString(15, 50, "YOU WIN!", BLACK);
//         lcd_writeFrame();
//         while (true)
//         {
//             vTaskDelay(GAME_END);
//         }
//     }
// }


void game_init(void)
{
    movement_init(&good_guy);
    attack_init(&good_guy);
    com_init(); // waits until connected
    
    // Store initial position
    good_prev_x = good_guy.x_loc;
    bad_prev_x = good_guy.x_loc;
    good_prev_y = good_guy.y_loc;
    bad_prev_y = good_guy.y_loc;

good_guy.health = MAX_HEALTH;
    good_guy.is_bad_guy = false;    // I am the good guy
    good_guy.hurt_timer = 0;

    bad_guy.health = MAX_HEALTH; 
    bad_guy.is_bad_guy = true;      // Opponent is bad guy
    bad_guy.hurt_timer = 0;
}

// tick helper functions
int32_t get_player_height(player_t *player)
{
    if (player->ducking)
    {
        return HURTBOX_HEIGHT/2;
    }
    return HURTBOX_HEIGHT;
}

void draw_health_bar(coord_t x, coord_t y, int16_t current, int16_t max)
{
    // Prevent overflow/underflow visual glitches
    if (current > max) current = max;
    
    // Calculate width of the "alive" part
    // Formula: (Current / Max) * Total_Width
    coord_t filled_w = (coord_t)((current * HEALTH_BAR_W) / max);

    // 1. Draw the "Empty" background (shows damage taken)
    lcd_fillRect(x, y, HEALTH_BAR_W, HEALTH_BAR_H, COLOR_HEALTH_EMPTY);

    // 2. Draw the "Filled" foreground (shows remaining health)
    if (filled_w > 0) {
        lcd_fillRect(x, y, filled_w, HEALTH_BAR_H, COLOR_HEALTH_FILLED);
    }

    // 3. Draw a Border (optional, looks nicer)
    lcd_drawRect(x, y, HEALTH_BAR_W, HEALTH_BAR_H, COLOR_HEALTH_BORDER);
}

void game_tick(void)
{
    // ESP_LOGI(TAG, "(before receive) Bad Guy Health: %d", bad_guy.health);

    // Clear previous sprite position
    sprite_clear_player(&good_guy, good_prev_x, good_prev_y);
    sprite_clear_player(&bad_guy, bad_prev_x, bad_prev_y);

    // Decrement hurt timers
    if (good_guy.hurt_timer > 0) good_guy.hurt_timer--;
    if (bad_guy.hurt_timer > 0) bad_guy.hurt_timer--;

    // game_hit_registered(&good_guy, &bad_guy);

    // Track bad guy health changes to trigger hurt animation
    int16_t bad_prev_health = bad_guy.health;

    // receive packet
    if (-1 == com_read(&yourPacket, sizeof(yourPacket)))
    {
        // didn't recieve anything
        // set send flag to send
    }
    else
    {
        // only populate bad guy if we recieved data
        game_populate_bad_guy(&bad_guy, &yourPacket);

        // If bad guy lost health, trigger their hurt animation
        if (bad_guy.health < bad_prev_health) 
        {
            bad_guy.hurt_timer = 15;
        }
    }

    // ESP_LOGI(TAG, "(after receive) Bad Guy Health: %d", bad_guy.health);
    
    // calculate facing direction
    if (good_prev_x > bad_prev_x)
    {
        good_guy.facing = FACING_LEFT;
        bad_guy.facing = FACING_RIGHT;
    }
    else
    {
        good_guy.facing = FACING_RIGHT;
        bad_guy.facing = FACING_LEFT;
    }

    // Update good guy game state (unless good guy is in hit stun)
    // if (!game_hit_registered(&good_guy, &bad_guy))
    // {
    //     movement_tick(&good_guy, &bad_guy);
    //     attack_tick(&good_guy);
    // }

    game_hit_registered(&good_guy, &bad_guy);
    movement_tick(&good_guy, &bad_guy);
    attack_tick(&good_guy);
    
    // Draw new sprite
    sprite_draw_player(&good_guy);
    sprite_draw_player(&bad_guy);

    // Draw Health UI
    draw_health_bar(P1_HEALTH_X, HEALTH_BAR_Y, good_guy.health, MAX_HEALTH);
    draw_health_bar(P2_HEALTH_X, HEALTH_BAR_Y, bad_guy.health, MAX_HEALTH);
    
    // Update previous position
    good_prev_x = good_guy.x_loc;
    bad_prev_x = bad_guy.x_loc;
    good_prev_y = good_guy.y_loc;
    bad_prev_y = bad_guy.y_loc;

    // ESP_LOGI(TAG, "(populate) Bad Guy Health: %d", bad_guy.health);

    // get info ready to send
    game_populate_packet(&good_guy, &myPacket);

    // send packet
    if (-1 == com_write(&myPacket, sizeof(myPacket)))
    {
        // didn't send anything
    }
    
    // ESP_LOGI(TAG, "(Game over) Bad Guy Health: %d", bad_guy.health);

    // check for game over
    // game_over(good_guy.health, bad_guy.health);
}

bool player_is_moving(player_t *player)
{
    if (player->movement_state == MOVEMENT_STATIONARY)
    {
        return true;
    }
    return false;
}