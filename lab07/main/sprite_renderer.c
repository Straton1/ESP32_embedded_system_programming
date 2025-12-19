#include "sprite_renderer.h"
#include "sprite_idle.h"
#include "sprite_duck.h"
#include "sprite_punch.h"
#include "sprite_kick.h"
#include "sprite_jump.h"
#include "sprite_hurt.h"

// Bad Guy Sprites
#include "sprite_bad_idle.h"
#include "sprite_bad_duck.h"
#include "sprite_bad_punch.h"
#include "sprite_bad_kick.h"
#include "sprite_bad_jump.h"
#include "sprite_bad_hurt.h"

#include "attack.h"
#include "config.h"
#include <string.h>

// Background color for clearing (matches config.h)
#define SPRITE_BG_COLOR CONFIG_COLOR_BACKGROUND
#define TRANSPARENT_COLOR 0xFFFF

/**
 * @brief Draw a sprite with optional horizontal flip
 */
static void draw_sprite_with_flip(coord_t x, coord_t y, const color_t *sprite, 
                                   coord_t width, coord_t height, bool flip) {
    if (!flip) {
        // Draw normally - but skip transparent pixels
        for (coord_t row = 0; row < height; row++) {
            for (coord_t col = 0; col < width; col++) {
                color_t pixel = sprite[row * width + col];
                if (pixel != TRANSPARENT_COLOR) {
                    lcd_drawPixel(x + col, y + row, pixel);
                }
            }
        }
    } else {
        // Draw flipped horizontally - skip transparent pixels
        for (coord_t row = 0; row < height; row++) {
            for (coord_t col = 0; col < width; col++) {
                color_t pixel = sprite[row * width + col];
                if (pixel != TRANSPARENT_COLOR) {
                    lcd_drawPixel(x + (width - 1 - col), y + row, pixel);
                }
            }
        }
    }
}

/**
 * @brief Get sprite dimensions and data based on player state
 */
static void get_current_sprite(player_t *player, const color_t **sprite_data, 
                                coord_t *width, coord_t *height) {
    
    // 1. HURT STATE (Highest Priority)
    if (player->hurt_timer > 0) {
        if (player->is_bad_guy) {
            *sprite_data = sprite_bad_hurt;
            *width = SPRITE_BAD_HURT_W;
            *height = SPRITE_BAD_HURT_H;
        } else {
            *sprite_data = sprite_hurt;
            *width = SPRITE_HURT_W;
            *height = SPRITE_HURT_H;
        }
        return;
    }

    // 2. ATTACK STATE
    if (player->is_attacking) {
        switch (player->attack_state) {
            case ATTACK_PUNCH:
                if (player->is_bad_guy) {
                    *sprite_data = sprite_bad_punch;
                    *width = SPRITE_BAD_PUNCH_W;
                    *height = SPRITE_BAD_PUNCH_H;
                } else {
                    *sprite_data = sprite_punch;
                    *width = SPRITE_PUNCH_W;
                    *height = SPRITE_PUNCH_H;
                }
                return;
            
            case ATTACK_KICK:
                if (player->is_bad_guy) {
                    *sprite_data = sprite_bad_kick;
                    *width = SPRITE_BAD_KICK_W;
                    *height = SPRITE_BAD_KICK_H;
                } else {
                    *sprite_data = sprite_kick;
                    *width = SPRITE_KICK_W;
                    *height = SPRITE_KICK_H;
                }
                return;
        }
    }
    
    // 3. JUMP STATE
    if (player->jumping) {
        if (player->is_bad_guy) {
            *sprite_data = sprite_bad_jump;
            *width = SPRITE_BAD_JUMP_W;
            *height = SPRITE_BAD_JUMP_H;
        } else {
            *sprite_data = sprite_jump;
            *width = SPRITE_JUMP_W;
            *height = SPRITE_JUMP_H;
        }
        return;
    }

    // 4. DUCK STATE
    if (player->ducking) {
        if (player->is_bad_guy) {
            *sprite_data = sprite_bad_duck;
            *width = SPRITE_BAD_DUCK_W;
            *height = SPRITE_BAD_DUCK_H;
        } else {
            *sprite_data = sprite_duck;
            *width = SPRITE_DUCK_W;
            *height = SPRITE_DUCK_H;
        }
        return;
    }
    
    // 5. IDLE STATE (Default)
    if (player->is_bad_guy) {
        *sprite_data = sprite_bad_idle;
        *width = SPRITE_BAD_IDLE_W;
        *height = SPRITE_BAD_IDLE_H;
    } else {
        *sprite_data = sprite_idle;
        *width = SPRITE_IDLE_W;
        *height = SPRITE_IDLE_H;
    }
}

/**
 * @brief Draw the player sprite based on current state
 */
void sprite_draw_player(player_t *player) {
    const color_t *sprite_data;
    coord_t sprite_w, sprite_h;
    
    // Get the appropriate sprite
    get_current_sprite(player, &sprite_data, &sprite_w, &sprite_h);
    
    // Calculate draw position (sprites are drawn from top-left)
    // Player y_loc is at the feet, so we need to offset by height
    coord_t draw_x = player->x_loc;
    coord_t draw_y = player->y_loc - sprite_h;
    
    // Adjust X position if facing left (so sprite extends in correct direction)
    if (player->facing == FACING_LEFT) {
        // For attack sprites, we want the body to stay at the same position
        // but the extended limb to go left instead of right
        if (player->is_attacking) {
            draw_x = player->x_loc - (sprite_w - SPRITE_IDLE_W);
        }
    }
    
    // Draw the sprite with flip if facing left
    bool flip = (player->facing == FACING_LEFT);
    draw_sprite_with_flip(draw_x, draw_y, sprite_data, sprite_w, sprite_h, flip);
}

/**
 * @brief Clear the player sprite at a position
 */
void sprite_clear_player(player_t *player, coord_t prev_x, coord_t prev_y) {
    // const color_t *sprite_data;
    // coord_t sprite_w, sprite_h;
    
    // Get sprite dimensions (use max size to ensure full clear)
    // We'll clear the largest possible sprite area
    coord_t max_width = SPRITE_PUNCH_W;  // Largest sprite width
    coord_t max_height = SPRITE_IDLE_H;   // Largest sprite height
    
    coord_t draw_x = prev_x;
    coord_t draw_y = prev_y - max_height;
    
    // Clear the area
    lcd_fillRect(draw_x - max_width, draw_y, max_width * 2, max_height, SPRITE_BG_COLOR);
}
