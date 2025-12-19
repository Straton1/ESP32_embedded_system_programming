#!/usr/bin/env python3
"""
Convert PNG sprite images to C arrays in RGB565 format.
"""

from PIL import Image
import os

def rgb888_to_rgb565(r, g, b):
    """Convert 24-bit RGB to 16-bit RGB565."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)

def convert_png_to_c(png_filename, output_name, bg_color=(0, 4, 16)):
    """
    Convert PNG to C array in RGB565 format.
    Pixels matching bg_color will be marked as transparent (0x0000 with special handling).
    """
    # Open image
    img = Image.open(png_filename)
    img = img.convert('RGB')
    
    width, height = img.size
    pixels = list(img.getdata())
    
    # Convert to RGB565
    rgb565_data = []
    for r, g, b in pixels:
        # Check if this is the background color (transparent)
        if (r, g, b) == bg_color:
            rgb565_data.append(0xFFFF)  # Use white/max value as "transparent" marker
        else:
            rgb565_data.append(rgb888_to_rgb565(r, g, b))
    
    # Generate C files
    var_name = output_name.lower()
    macro_name = output_name.upper()
    
    # Write .h file
    with open(f"{output_name}.h", 'w') as f:
        f.write(f"#ifndef {macro_name}_H_\n")
        f.write(f"#define {macro_name}_H_\n\n")
        f.write("#include <stdint.h>\n")
        f.write('#include "lcd.h"\n\n')
        f.write(f"#define {macro_name}_W {width}\n")
        f.write(f"#define {macro_name}_H {height}\n")
        f.write(f"#define {macro_name}_PIXELS {width * height}\n\n")
        f.write(f"extern const color_t {var_name}[{macro_name}_PIXELS];\n\n")
        f.write(f"#endif // {macro_name}_H_\n")
    
    # Write .c file
    with open(f"{output_name}.c", 'w') as f:
        f.write(f'#include "{output_name}.h"\n\n')
        f.write(f"const color_t {var_name}[{macro_name}_PIXELS] = {{\n")
        
        # Write data in rows of 12 values
        for i in range(0, len(rgb565_data), 12):
            row = rgb565_data[i:i+12]
            f.write("    ")
            f.write(", ".join(f"0x{val:04X}" for val in row))
            if i + 12 < len(rgb565_data):
                f.write(",\n")
            else:
                f.write("\n")
        
        f.write("};\n")
    
    print(f"Converted {png_filename} -> {output_name}.c/.h ({width}x{height})")

if __name__ == "__main__":
    # Background color matching config.h
    BG = (0, 4, 16)
    
    # Convert each sprite
    sprites = [
        # Good Guy
        ("image/sprite_idle.png", "sprite_idle"),
        ("image/sprite_duck.png", "sprite_duck"),
        ("image/sprite_punch.png", "sprite_punch"),
        ("image/sprite_kick.png", "sprite_kick"),
        ("image/sprite_jump.png", "sprite_jump"),
        ("image/sprite_hurt.png", "sprite_hurt"),

        # Bad Guy
        ("image/sprite_bad_idle.png", "sprite_bad_idle"),
        ("image/sprite_bad_duck.png", "sprite_bad_duck"),
        ("image/sprite_bad_punch.png", "sprite_bad_punch"),
        ("image/sprite_bad_kick.png", "sprite_bad_kick"),
        ("image/sprite_bad_jump.png", "sprite_bad_jump"),
        ("image/sprite_bad_hurt.png", "sprite_bad_hurt"),
    ]
    
    for png_file, output_name in sprites:
        if os.path.exists(png_file):
            convert_png_to_c(png_file, output_name, BG)
        else:
            print(f"Warning: {png_file} not found")
    
    print("\nDone! Add the .c files to your CMakeLists.txt")
