#include <stdlib.h> 
#include <math.h>
#include "tone.h"

#define BUFFER_DIVISOR 20       // Divisor to determine buffer size from sample rate
#define DAC_MAX_VAL 255         // The maximum value for the DAC output
#define DAC_MID_VAL 128         // The midpoint (offset) for the DAC output
#define DAC_AMPLITUDE 127       // The amplitude of the waveform (max value - mid value)
#define PI_FLOAT 3.14159f       // Define PI for float calculations
#define TWO_PI_FLOAT (2.0f * PI_FLOAT)
#define DAC_MAX_VAL_FLOAT 255.0f         // The maximum value for the DAC output
#define DAC_MID_VAL_FLOAT 128.0f         // The midpoint (offset) for the DAC output
#define DAC_AMPLITUDE_FLOAT 127.0f       // The amplitude of the waveform (max value - mid value)
#define WAVE_QUARTER_DIVISOR 4
#define TRIANGLE_WAVE_THIRD_QUARTER 3

// A global pointer for our waveform data buffer
static uint8_t *g_waveform_buffer = NULL;
static uint32_t g_sample_rate = 0;

// Initializes the sound generation component and allocates the waveform buffer.
int32_t tone_init(uint32_t sample_hz)
{
    // Set min sample rate
    uint32_t min_sample_rate = (2 * LOWEST_FREQ);

    // Validate sample rate
    if (sample_hz < min_sample_rate)
    {
        return -1;
    } 

    // Add sample_hz to global variable
    g_sample_rate = sample_hz;

    // Init sound component
    sound_init(sample_hz);
    
    // Buffer size
    uint32_t buffer_size = sample_hz / 20;
    
    // Allocate buffer 
    g_waveform_buffer = (uint8_t *)malloc(buffer_size * sizeof(uint8_t));
    
    // Check if the allocation was successful
    if (g_waveform_buffer == NULL) 
    {
        return -1;
    }

    return 0;
}


// Frees the waveform buffer and deinitializes the sound component.
int32_t tone_deinit(void)
{
        // Check if the pointer is valid before freeing
    if (g_waveform_buffer != NULL) 
    {
        free(g_waveform_buffer);
        // Set the pointer to NULL to prevent it from being used again
        g_waveform_buffer = NULL;
    }

    sound_deinit();
    
    return 0;
}


// Generates a waveform of a specific type and frequency and starts cyclic output.
void tone_start(tone_t tone, uint32_t freq)
{
    // Check the parameters are in bounds
    if ((freq < LOWEST_FREQ) || (freq > (g_sample_rate / 2))) 
    {
        return;
    }

    // Check if tone type is invalid
    if (tone >= LAST_T) {
        return;
    }

        uint32_t samples_per_period = g_sample_rate / freq;

    // Generate the waveform based on the selected tone type.
    switch (tone)
    {
        case SINE_T:
                // Generate a sine wave period.
                for (uint32_t i = 0; i < samples_per_period; i++)
                {
                    // Calculate the angle for this sample
                    float angle = ((float)i / (samples_per_period)) * TWO_PI_FLOAT;
                    // Calculate the SIN value (-1.0, 1.0)
                    float sin_value = sinf(angle);
                    // Find the associated DAC value
                    g_waveform_buffer[i] = (uint8_t)(DAC_MID_VAL + (DAC_AMPLITUDE * sin_value));
                }
            break;
        // Generate a square wave period.
        case SQUARE_T:
            for (uint32_t i = 0; i < samples_per_period; i++)
            {
                if (i < samples_per_period / 2)
                {
                    g_waveform_buffer[i] = DAC_MAX_VAL;
                }
                else
                {
                    g_waveform_buffer[i] = 0;
                }
            }
            break;
        // Generate a triangle wave period.
        case TRIANGLE_T:
            uint32_t quarter_period = samples_per_period / WAVE_QUARTER_DIVISOR;
            if (quarter_period == 0) 
            { 
                quarter_period = 1; 
            }
            uint32_t half_period_tri = samples_per_period / 2;
            uint32_t three_quarter_period = samples_per_period * TRIANGLE_WAVE_THIRD_QUARTER / WAVE_QUARTER_DIVISOR;

            for (uint32_t i = 0; i < samples_per_period; i++) 
            {
                if (i < quarter_period) 
                {
                    // Ramp up from 128 to 255
                    g_waveform_buffer[i] = DAC_MID_VAL + (uint8_t)((DAC_AMPLITUDE_FLOAT / quarter_period) * i);
                } 
                else if (i < three_quarter_period) 
                {
                    // Ramp down from 255 to 0
                    g_waveform_buffer[i] = DAC_MAX_VAL - (uint8_t)((DAC_MAX_VAL_FLOAT / half_period_tri) * (i - quarter_period));
                }
                else 
                {
                    // Ramp up from 0 to 128
                    g_waveform_buffer[i] = (uint8_t)((DAC_MID_VAL_FLOAT / quarter_period) * (i - three_quarter_period));
                }
            }
            break;
        // Generate a sawtooth wave period.
        case SAW_T:
            uint32_t half_period_saw = samples_per_period / 2;

            for (uint32_t i = 0; i < samples_per_period; i++)
            {
                if (i < half_period_saw) {
                    // Ramp from 128 up to 255
                    g_waveform_buffer[i] = DAC_MID_VAL + (uint8_t)((DAC_AMPLITUDE_FLOAT / half_period_saw) * i);
                } 
                else 
                {
                    // Ramp from 0 up to 128
                    g_waveform_buffer[i] = (uint8_t)((DAC_MID_VAL_FLOAT / half_period_saw) * (i - half_period_saw));
                
                }
            }
            break;

        default:
            break;
    }

    sound_cyclic(g_waveform_buffer, samples_per_period);
}



