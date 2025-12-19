#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "game.h"
#include "hw.h"
#include "lcd.h"
#include "sound.h"
#include "pin.h"
#include "config.h"
#include "cursor.h"
#include "joy.h"
#include "uart.h"

// include sound support

static const char *TAG = "lab07";

void draw_floor(void);

extern player_t good_guy;
extern player_t bad_guy;

// Define states for the main application
typedef enum {
    STATE_START_SCREEN,
    STATE_PLAYING,
	STATE_GAME_OVER
} app_state_e;

// The update period as an integer in ms
#define PER_MS ((uint32_t)(CONFIG_GAME_TIMER_PERIOD*1000))
#define TIME_OUT 500 // ms

#define CURSOR_SZ 7 // Cursor size (width & height) in pixels

#define CHK_RET(x) ({                                           \
        int32_t ret_val = (x);                                  \
        if (ret_val != 0) {                                     \
            ESP_LOGE(TAG, "FAIL: return %ld, %s", ret_val, #x); \
        }                                                       \
        ret_val;                                                \
    })

TimerHandle_t update_timer; // Declare timer handle for update callback

volatile bool interrupt_flag;

uint32_t isr_triggered_count;
uint32_t isr_handled_count;

// Interrupt handler for game - use flag method
void update() {
	interrupt_flag = true;
	isr_triggered_count++;
}

void draw_floor(void) {
    // Draw a filled rectangle from GROUND_LEVEL down to the bottom of the screen
    // Arguments: x, y, width, height, color
    lcd_fillRect(0, GROUND_LEVEL, HW_LCD_W, HW_LCD_H - GROUND_LEVEL, GROUND_COLOR);
    
    // Optional: Draw a line on top to define the edge clearly
    lcd_drawHLine(0, GROUND_LEVEL, HW_LCD_W, WHITE); 
}

// Main application
void app_main(void)
{
	ESP_LOGI(TAG, "Starting");

	// ISR flag and counts
	interrupt_flag = false;
	isr_triggered_count = 0;
	isr_handled_count = 0;

	// Initialization
	joy_init();
	lcd_init();
	lcd_frameEnable();
	lcd_fillScreen(CONFIG_COLOR_BACKGROUND);
	game_init(); // Initializes UART, Pins, and data structures

	// Configure I/O pins for buttons
	pin_reset(HW_BTN_A);
	pin_input(HW_BTN_A, true);
	pin_reset(HW_BTN_B);
	pin_input(HW_BTN_B, true);
	pin_reset(HW_BTN_MENU);
	pin_input(HW_BTN_MENU, true);
	pin_reset(HW_BTN_OPTION);
	pin_input(HW_BTN_OPTION, true);
	pin_reset(HW_BTN_SELECT);
	pin_input(HW_BTN_SELECT, true);
	pin_reset(HW_BTN_START);
	pin_input(HW_BTN_START, true);

	// Initialize update timer
	update_timer = xTimerCreate(
		"update_timer",        // Text name for the timer.
		pdMS_TO_TICKS(PER_MS), // The timer period in ticks.
		pdTRUE,                // Auto-reload the timer when it expires.
		NULL,                  // No need for a timer ID.
		update                 // Function called when timer expires.
	);
	if (update_timer == NULL) {
		ESP_LOGE(TAG, "Error creating update timer");
		return;
	}
	if (xTimerStart(update_timer, pdMS_TO_TICKS(TIME_OUT)) != pdPASS) {
		ESP_LOGE(TAG, "Error starting update timer");
		return;
	}

    // Initial State
    app_state_e current_state = STATE_START_SCREEN;

    // Buffers for manual packet syncing in Start Screen
    player_net_state_t tx_packet;
    player_net_state_t rx_packet;

	// Main game loop
	uint64_t t1, t2, tmax = 0; // For hardware timer values
	
    while (1) // Loop forever
	{
		// Wait for Timer Interrupt
        while (!interrupt_flag) {}
		t1 = esp_timer_get_time();
		interrupt_flag = false;
		isr_handled_count++;

        // ====================================================================
        // STATE: START SCREEN
        // ====================================================================
        if (current_state == STATE_START_SCREEN)
        {
            // Clear Screen every frame (or optimize to clear once, but this is safer for text updates)
            lcd_fillScreen(CONFIG_COLOR_BACKGROUND);

			draw_floor();

            // 1. Draw Title Text
            lcd_setFontSize(2);
            lcd_drawString(30, 40, "PRESS START", WHITE);

            // 2. Handle Input (Start Button)
            // Active Low: 0 means pressed
            if (pin_get_level(HW_BTN_START) == 0) 
            {
                good_guy.ready_to_start = true;
            }

            // 3. Draw Status
            if (good_guy.ready_to_start) {
                lcd_setFontSize(1);
                lcd_drawString(30, 80, "WAITING FOR P2...", YELLOW);
            }
            if (bad_guy.ready_to_start) {
                lcd_setFontSize(1);
                lcd_drawString(30, 100, "PLAYER 2 READY!", GREEN);
            }

            // 4. Manual Network Sync (Since game_tick is not running)
            // Populate our packet
            game_populate_packet(&good_guy, &tx_packet);
            
            // Send our packet
            com_write(&tx_packet, sizeof(tx_packet));

            // Receive opponent packet
            if (com_read(&rx_packet, sizeof(rx_packet)) != -1)
            {
                // Update enemy state (specifically the ready_to_start flag)
                game_populate_bad_guy(&bad_guy, &rx_packet);
            }

            // 5. State Transition
            // If both players are ready, reset the game and start
            if (good_guy.ready_to_start && bad_guy.ready_to_start)
            {
                game_reset_round(); // Reset health, positions, and flags
                current_state = STATE_PLAYING;
                lcd_fillScreen(CONFIG_COLOR_BACKGROUND); // Clear before game starts
				draw_floor();
            }
            
            // Push frame to LCD
            lcd_writeFrame();
        }

        // ====================================================================
        // STATE: PLAYING
        // ====================================================================
        else if (current_state == STATE_PLAYING)
        {
            // 1. Check for Reset Request (SELECT button)
            if (pin_get_level(HW_BTN_SELECT) == 0)
            {
                good_guy.wants_reset = true;
            }

            // 2. Check for Reset Acceptance (OPTION button)
            // Only valid if opponent has requested a reset
            if (bad_guy.wants_reset && pin_get_level(HW_BTN_OPTION) == 0)
            {
                good_guy.accept_reset = true;
            }

            // 3. Run Game Logic (Physics, Movement, Networking)
            // game_tick() handles com_write/com_read internally
            game_tick(); 

			// Check if game over
			if (good_guy.health <= 0 || bad_guy.health <= 0)
            {
                current_state = STATE_GAME_OVER;
            }

            // 4. Draw Reset UI Overlays
            if (bad_guy.wants_reset)
            {
                lcd_setFontSize(1);
                lcd_drawString(5, 25, "OPPONENT WANTS RESET", RED);
                lcd_drawString(5, 35, "PRESS OPTION TO ACCEPT", WHITE);
            }
            else if (good_guy.wants_reset)
            {
                lcd_setFontSize(1);
                lcd_drawString(5, 25, "WAITING FOR ACCEPT...", YELLOW);
            }

            // 5. Push frame to LCD
            lcd_writeFrame();

            // 6. Handle Reset Transition
            // Check this AFTER game_tick so the "accept" packet has a chance to be sent
            if (good_guy.accept_reset || bad_guy.accept_reset)
            {
                game_reset_round(); // Clear all flags and health
                current_state = STATE_START_SCREEN;
            }
        }
		// ====================================================================
        // STATE: GAME OVER
        // ====================================================================
        else if (current_state == STATE_GAME_OVER)
        {
            // 1. Determine Result
            bool i_lost = (good_guy.health <= 0);
            
            // 2. Draw Screen
            if (i_lost) {
                lcd_fillScreen(RED);
                lcd_setFontSize(2);
                lcd_drawString(60, 80, "YOU LOSE", WHITE);
            } else {
                lcd_fillScreen(GREEN);
                lcd_setFontSize(2);
                lcd_drawString(60, 80, "YOU WIN!", BLACK);
            }

            lcd_setFontSize(1);
            lcd_drawString(40, 130, "PRESS START TO RESET", WHITE);

            // 3. Handle Input
            if (pin_get_level(HW_BTN_START) == 0)
            {
                game_reset_round();
                current_state = STATE_START_SCREEN;
            }
            
            lcd_writeFrame();
        }
        // Timing Calculation
		t2 = esp_timer_get_time() - t1;
		if (t2 > tmax) tmax = t2;
	}
    
	printf("Handled %lu of %lu interrupts\n", isr_handled_count, isr_triggered_count);
	printf("WCET us:%llu\n", tmax);
}