#ifndef SPRITE_RENDERER_H_
#define SPRITE_RENDERER_H_

#include "game.h"
#include "lcd.h"

// static void draw_sprite_with_flip(coord_t x, coord_t y, const color_t *sprite, coord_t width, coord_t height, bool flip);

// static void get_current_sprite(player_t *player, const color_t **sprite_data, coord_t *width, coord_t *height);

/**
 * @brief Draw the player sprite based on current state
 * @param player Pointer to player structure
 */
void sprite_draw_player(player_t *player);

/**
 * @brief Clear the player sprite at the previous position
 * @param player Pointer to player structure
 * @param prev_x Previous X position
 * @param prev_y Previous Y position
 */
void sprite_clear_player(player_t *player, coord_t prev_x, coord_t prev_y);

#endif // SPRITE_RENDERER_H_
