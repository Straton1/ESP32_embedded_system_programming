#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

// GPIO Matrix Registers
#define GPIO_OUT_REG		 (DR_REG_GPIO_BASE + 0x04)
#define GPIO_OUT1_REG 		 (DR_REG_GPIO_BASE + 0x10)
#define GPIO_IN_REG       	 (DR_REG_GPIO_BASE + 0x3C)
#define GPIO_IN1_REG      	 (DR_REG_GPIO_BASE + 0x40)
#define GPIO_ENABLE_REG      (DR_REG_GPIO_BASE + 0x20)
#define GPIO_ENABLE1_REG     (DR_REG_GPIO_BASE + 0x2C)

// IO Pin Registers
#define GPIO_PIN_REG(n) 	 (DR_REG_GPIO_BASE + 0xAC + (n * 4))
// This is the shortened version of GPIO_OUT_FUNC_SEL_CFG_REGn
#define FUNC_REG(n) 		 (DR_REG_GPIO_BASE + 0x530 + (n * 4)) 

// FUNC_REG reset value
#define FUNC_RESET_VAL  0x100

// IO MUX Registers	
#define IO_MUX_REG(n) 		 (DR_REG_IO_MUX_BASE + PIN_MUX_REG_OFFSET[n])

// IO MUX Register Fields - FUN_WPD, FUN_WPU, ...
#define FUN_WPD  		7
#define FUN_WPU  		8
#define FUN_IE	 		9
#define FUN_DRV			10 // bits 10 and 11
#define MCU_SEL			12 // bits 12-14
#define PAD_DRIVER_BIT  2

// Helpful Register macros
#define REG(r) (*(volatile uint32_t *)(r))
#define REG_BITS 32
#define REG_SET_BIT(r,b) (REG(r) |= (1 << b))
#define REG_CLR_BIT(r,b) (REG(r) &= ~(1 << b))
#define REG_GET_BIT(r,b) ((REG(r) & (1 << b)) >> b)

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};


// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
// Return zero if successful, or non-zero otherwise.
int32_t pin_reset(pin_num_t pin)
{
	
	// Delegate to the RTC subsystem if it's an RTC-capable pin.
    if (rtc_gpio_is_valid_gpio(pin)) 
	{ 
        rtc_gpio_deinit(pin);
        rtc_gpio_pullup_en(pin);
        rtc_gpio_pulldown_dis(pin);
    }

    // 1. Reset the PIN register to all zeros to disable open-drain, etc.
    REG(GPIO_PIN_REG(pin)) = 0;

    // 2. Reset the FUNC register to 0x100 to select simple GPIO.
    REG(FUNC_REG(pin)) = FUNC_RESET_VAL;

    // 3. Reset the IO_MUX register to a safe default.
	// This sets MCU_SEL=2, FUN_DRV=2, enables input (FUN_IE=1) and weak pull-up (FUN_WPU=1).
    uint32_t mux_val = (2 << MCU_SEL) | (2 << FUN_DRV) | (1 << FUN_IE) | (1 << FUN_WPU);
    REG(IO_MUX_REG(pin)) = mux_val;

    // Now that the pin is reset, set its output level to zero.
    return 0;
}

// Enable or disable a pull-up on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) // hand-off work to RTC subsystem
	{ 									
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	// Enable or disable the pull-up based on the enable flag.
	if (enable) 
	{
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	} 
	else 
	{
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	
	return 0;
}

// Enable or disable a pull-down on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	// Delegate to the RTC subsystem if it's an RTC-capable pin.
	if (rtc_gpio_is_valid_gpio(pin)) 
	{ 
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	// Enable or disable the pull-down based on the enable flag.
	if (enable) 
	{
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
	} 
	else 
	{
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	return 0;
}

// Enable or disable the pin as an input signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_input(pin_num_t pin, bool enable)
{
	// Set or clear the FUN_IE bit in an IO_MUX register
	if (enable) 
	{
		REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
	} 
	else 
	{
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	return 0;
}

// Enable or disable the pin as an output signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_output(pin_num_t pin, bool enable)
{
	// Set or clear the I/O pin bit in the ENABLE or ENABLE1 register
	// Use the first GPIO enable register for pins 0-31.
	if (pin < REG_BITS)
	// Check which register to use based on the pin number
	{
		if (enable) 
		{
		REG_SET_BIT(GPIO_ENABLE_REG, pin);
		} 
		else 
		{
		REG_CLR_BIT(GPIO_ENABLE_REG, pin);
		}
	}
	// Use the second register for pins 32 and up
	else
	{
		if (enable) 
		{
		REG_SET_BIT(GPIO_ENABLE1_REG, (pin - REG_BITS));
		} 
		else 
		{
		REG_CLR_BIT(GPIO_ENABLE1_REG, (pin - REG_BITS));
		}
	}
	
	return 0;
}

// Enable or disable the pin as an open-drain signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	// Set or clear the PAD_DRIVER bit in a PIN register
	// Enable or disable open-drain based on the enable flag.
	if (enable) 
	{
		// If true, set the PAD_DRIVER_BIT in the correct PIN register.
		REG_SET_BIT(GPIO_PIN_REG(pin), PAD_DRIVER_BIT);
	} 
	else 
	{
		// If false, clear the PAD_DRIVER_BIT in the correct PIN register.
		REG_CLR_BIT(GPIO_PIN_REG(pin), PAD_DRIVER_BIT);
	}

	return 0;
}

// Sets the output signal level if the pin is configured as an output.
// Return zero if successful, or non-zero otherwise.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	// Set or clear the I/O pin bit in the OUT or OUT1 register
	// Use the first GPIO enable register for pins 0-31.
	if (pin < REG_BITS)
	{
		if (level)
		{
			REG_SET_BIT(GPIO_OUT_REG,(pin));	
		}
		else
		{
			REG_CLR_BIT(GPIO_OUT_REG,(pin));
		}
	}
	// Use the second register for pins 32 and up
	else
	{
		if (level)
		{
			REG_SET_BIT(GPIO_OUT1_REG,(pin-REG_BITS));	
		}
		else
		{
			REG_CLR_BIT(GPIO_OUT1_REG,(pin-REG_BITS));
		}
	}
	return 0;
}

// Gets the input signal level if the pin is configured as an input.
// Return zero or one if successful, or negative otherwise.
int32_t pin_get_level(pin_num_t pin)
{
	// Get the I/O pin bit from the IN or IN1 register
	return (pin < REG_BITS) ? REG_GET_BIT(GPIO_IN_REG, pin) : REG_GET_BIT(GPIO_IN1_REG, (pin - REG_BITS));
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
	// Read the 32-bit values from both input registers.
    uint32_t high_bits = REG(GPIO_IN1_REG);
    uint32_t low_bits = REG(GPIO_IN_REG);

	// Cast the high bits to 64-bit, shift it left by 32,
    // and combine it with the low bits.
    return ((uint64_t)high_bits << REG_BITS) | low_bits; 
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
	// Read the 32-bit values from both output registers.
    uint32_t high_bits = REG(GPIO_OUT1_REG);
    uint32_t low_bits = REG(GPIO_OUT_REG);

	// Cast the high bits to 64-bit, shift it left by 32,
    // and combine it with the low bits.
    return ((uint64_t)high_bits << REG_BITS) | low_bits; 
}
