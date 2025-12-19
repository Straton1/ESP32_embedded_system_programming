#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "hw.h"
#include "lcd.h"
#include "uart.h" 

// UART Configuration
#define UART_PORT_NUM      UART_NUM_1
#define UART_BAUD_RATE     115200
#define BUF_SIZE           1024

// Map to External Header pins defined in hw_gc.h
#define UART_TX_PIN        HW_EX8
#define UART_RX_PIN        HW_EX7

// Initialize the communication channel.
int32_t com_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver
    esp_err_t ret = uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
    if (ret != ESP_OK) return 1;

    // Configure UART parameters
    ret = uart_param_config(UART_PORT_NUM, &uart_config);
    if (ret != ESP_OK) return 1;

    // Set UART pins
    ret = uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) return 1;
    
    return 0; 
}

// Free resources used for communication.
int32_t com_deinit(void)
{
    esp_err_t ret = uart_driver_delete(UART_PORT_NUM);
    if (ret != ESP_OK) return 1;
    return 0;
}

// Write data to the communication channel.
int32_t com_write(const void *buf, uint32_t size)
{
    int txBytes = uart_write_bytes(UART_PORT_NUM, (const char*)buf, size);
    if (txBytes < 0) {
        return -1;
    }
    return txBytes;
}

// Read data from the communication channel.
int32_t com_read(void *buf, uint32_t size)
{
    // Read data from UART.
    int rxBytes = uart_read_bytes(UART_PORT_NUM, buf, size, 0);
    
    // FIX: If we read 0 bytes (no data) or fewer bytes than expected,
    // return -1 to tell game.c that no valid packet was received.
    if (rxBytes <= 0 || (uint32_t)rxBytes != size) {
        return -1; 
    }

    return rxBytes;
}