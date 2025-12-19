#include "esp_adc/adc_oneshot.h"
#include "joy.h"

// Take 64 samples for a stable average
#define NUM_SAMPLES 64

// Handle for ADC Unit 1
static adc_oneshot_unit_handle_t g_adc_handle;

// Calibrated center position of the joystick
static int32_t g_x_center = 0;
static int32_t g_y_center = 0;

// Init function for the joystick
int32_t joy_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,           
        .ulp_mode = ADC_ULP_MODE_DISABLE 
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &g_adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
    .bitwidth = ADC_BITWIDTH_DEFAULT, // Use default bitwidth (12-bit)
    .atten = ADC_ATTEN_DB_12          // Attenuate input by 12 dB
    };
    // Configure ADC Channel 6 (X-axis)
    ESP_ERROR_CHECK(adc_oneshot_config_channel(g_adc_handle, ADC_CHANNEL_6, &chan_cfg));

    // Configure ADC Channel 7 (Y-axis)
    ESP_ERROR_CHECK(adc_oneshot_config_channel(g_adc_handle, ADC_CHANNEL_7, &chan_cfg));


    int_fast32_t x_total = 0;
    int_fast32_t y_total = 0;

    // Sample the ADC multiple times to find a stable center point.
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) 
    {
        int_fast32_t x_raw, y_raw; // Variables to store a single reading
        ESP_ERROR_CHECK(adc_oneshot_read(g_adc_handle, ADC_CHANNEL_6, &x_raw));
        ESP_ERROR_CHECK(adc_oneshot_read(g_adc_handle, ADC_CHANNEL_7, &y_raw));
        x_total += x_raw;
        y_total += y_raw;
    }
    
    g_x_center = x_total / NUM_SAMPLES;
    g_y_center = y_total / NUM_SAMPLES;

    return 0;
}

// Deinitializes and frees the ADC unit resource.
int32_t joy_deinit(void)
{
    if (g_adc_handle != NULL)
    {
    ESP_ERROR_CHECK(adc_oneshot_del_unit(g_adc_handle));
    }
    
    return 0;
}

// Reads the joystick's current ADC values and calculates the displacement from the calibrated center.
void joy_get_displacement(int32_t *dcx, int32_t *dcy)
{
    // Variables to store the raw ADC reading
    int_fast32_t x_raw, y_raw;

    // Read the current position of the joystick
    ESP_ERROR_CHECK(adc_oneshot_read(g_adc_handle, ADC_CHANNEL_6, &x_raw));
    ESP_ERROR_CHECK(adc_oneshot_read(g_adc_handle, ADC_CHANNEL_7, &y_raw));

    // Calculate displacement relative to the calibrated center and store it
    // using the pointers passed into the function.
    *dcx = x_raw - g_x_center;
    *dcy = y_raw - g_y_center;
}