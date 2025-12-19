import os

# --- Configuration ---
# Colors (R, G, B)
BG_COLOR = (0, 4, 16)       # Background (matches config.h)
SKIN_COLOR = (255, 200, 150)
GLOVE_COLOR = (255, 0, 0)   # Red gloves/shoes

# Character Themes
COLOR_GOOD = (0, 255, 0)    # Green
COLOR_BAD = (180, 0, 255)   # Purple

# --- Virtual Canvas ---
class Canvas:
    def __init__(self, width, height, bg_color):
        self.width = width
        self.height = height
        self.bg_color = bg_color
        # Initialize grid with background color
        self.pixels = [[bg_color for _ in range(width)] for _ in range(height)]

    def draw_rect(self, x1, y1, x2, y2, color):
        """Draws a filled rectangle from (x1,y1) to (x2,y2) inclusive."""
        for y in range(y1, y2 + 1):
            for x in range(x1, x2 + 1):
                if 0 <= x < self.width and 0 <= y < self.height:
                    self.pixels[y][x] = color

    def draw_line(self, x1, y1, x2, y2, color):
        """Simple Bresenham's line algorithm for visual effects."""
        dx = abs(x2 - x1)
        dy = abs(y2 - y1)
        sx = 1 if x1 < x2 else -1
        sy = 1 if y1 < y2 else -1
        err = dx - dy
        
        while True:
            if 0 <= x1 < self.width and 0 <= y1 < self.height:
                self.pixels[y1][x1] = color
            if x1 == x2 and y1 == y2:
                break
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x1 += sx
            if e2 < dx:
                err += dx
                y1 += sy

# --- C Code Generation ---
def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)

def save_as_c(canvas, output_name):
    var_name = output_name.lower()
    macro_name = output_name.upper()
    
    # 1. Write Header (.h)
    with open(f"{output_name}.h", "w") as f:
        f.write(f"#ifndef {macro_name}_H_\n")
        f.write(f"#define {macro_name}_H_\n\n")
        f.write("#include <stdint.h>\n")
        f.write('#include "lcd.h"\n\n')
        f.write(f"#define {macro_name}_W {canvas.width}\n")
        f.write(f"#define {macro_name}_H {canvas.height}\n")
        f.write(f"#define {macro_name}_PIXELS {canvas.width * canvas.height}\n\n")
        f.write(f"extern const color_t {var_name}[{macro_name}_PIXELS];\n\n")
        f.write(f"#endif // {macro_name}_H_\n")

    # 2. Write Source (.c)
    with open(f"{output_name}.c", "w") as f:
        f.write(f'#include "{output_name}.h"\n\n')
        f.write(f"const color_t {var_name}[{macro_name}_PIXELS] = {{\n")
        
        count = 0
        line = []
        for y in range(canvas.height):
            for x in range(canvas.width):
                r, g, b = canvas.pixels[y][x]
                
                # Handle Transparency (Background color becomes 0xFFFF)
                if (r, g, b) == BG_COLOR:
                    val = 0xFFFF
                else:
                    val = rgb888_to_rgb565(r, g, b)
                
                line.append(f"0x{val:04X}")
                
                if len(line) >= 12:
                    f.write("    " + ", ".join(line) + ",\n")
                    line = []

        if line:
            f.write("    " + ", ".join(line) + "\n")
        
        f.write("};\n")
    
    print(f"Generated {output_name}.c/.h")

# --- Sprite Definitions ---
def draw_sprite(canvas, action, body_color):
    c = canvas
    # "Good Guy" vs "Bad Guy" logic is handled by passing 'body_color'
    
    if action == "idle":
        # Head
        c.draw_rect(6, 4, 13, 11, SKIN_COLOR)
        # Torso
        c.draw_rect(5, 12, 14, 35, body_color)
        # Legs
        c.draw_rect(5, 36, 8, 59, body_color)
        c.draw_rect(11, 36, 14, 59, body_color)
        # Arms (at sides)
        c.draw_rect(2, 12, 4, 30, SKIN_COLOR)
        c.draw_rect(15, 12, 17, 30, SKIN_COLOR)

    elif action == "duck":
        # Head (squatted down)
        c.draw_rect(6, 2, 13, 9, SKIN_COLOR)
        # Torso (Compressed)
        c.draw_rect(5, 10, 14, 22, body_color)
        # Legs (Crouched)
        c.draw_rect(4, 23, 15, 29, body_color)
        # Arms (Guarding)
        c.draw_rect(15, 8, 18, 18, SKIN_COLOR)

    elif action == "punch":
        # Head
        c.draw_rect(6, 4, 13, 11, SKIN_COLOR)
        # Torso
        c.draw_rect(5, 12, 14, 35, body_color)
        # Legs (Braced)
        c.draw_rect(5, 36, 8, 59, body_color) 
        c.draw_rect(11, 36, 18, 59, body_color) 
        # Back Arm
        c.draw_rect(4, 14, 6, 25, SKIN_COLOR)
        # PUNCHING ARM (Extends right)
        c.draw_rect(14, 14, 25, 18, SKIN_COLOR)
        c.draw_rect(25, 14, 42, 20, GLOVE_COLOR)

    elif action == "kick":
        # Head
        c.draw_rect(6, 4, 13, 11, SKIN_COLOR)
        # Torso (Leaning back)
        c.draw_rect(3, 12, 12, 35, body_color)
        # Left Leg (Standing)
        c.draw_rect(5, 36, 9, 59, body_color)
        # KICKING LEG (Extends right and up)
        c.draw_rect(12, 30, 25, 36, body_color)
        c.draw_rect(25, 25, 43, 32, GLOVE_COLOR)
        # Arms (Balance)
        c.draw_rect(2, 15, 4, 25, SKIN_COLOR)

    elif action == "jump":
        # Head
        c.draw_rect(6, 2, 13, 9, SKIN_COLOR)
        # Torso
        c.draw_rect(5, 10, 14, 33, body_color)
        # Legs (Tucked up)
        c.draw_rect(4, 34, 15, 45, body_color)
        # Shoes (Dangling)
        c.draw_rect(4, 45, 7, 50, GLOVE_COLOR)
        c.draw_rect(12, 45, 15, 50, GLOVE_COLOR)
        # Arms (Flailing up)
        c.draw_rect(1, 8, 3, 25, SKIN_COLOR)
        c.draw_rect(16, 8, 18, 25, SKIN_COLOR)

    elif action == "hurt":
        # Head (Knocked back)
        c.draw_rect(4, 4, 11, 11, SKIN_COLOR)
        # Torso (Leaning back)
        c.draw_rect(6, 12, 15, 35, body_color)
        # Legs (Staggering)
        c.draw_rect(2, 36, 6, 59, body_color)
        c.draw_rect(10, 36, 14, 55, body_color)
        # Arms (Thrown up in shock)
        c.draw_rect(0, 10, 2, 28, SKIN_COLOR)
        c.draw_rect(16, 8, 18, 26, SKIN_COLOR)
        # Visual Flash (White X on chest)
        c.draw_line(8, 15, 13, 25, (255, 255, 255))
        c.draw_line(13, 15, 8, 25, (255, 255, 255))

# --- Main Execution ---
if __name__ == "__main__":
    
    tasks = [
        # (Filename, Width, Height, Action, Color)
        ("sprite_idle", 20, 60, "idle", COLOR_GOOD),
        ("sprite_duck", 20, 30, "duck", COLOR_GOOD),
        ("sprite_punch", 45, 60, "punch", COLOR_GOOD),
        ("sprite_kick", 45, 60, "kick", COLOR_GOOD),
        ("sprite_jump", 20, 60, "jump", COLOR_GOOD),
        ("sprite_hurt", 20, 60, "hurt", COLOR_GOOD),
        
        ("sprite_bad_idle", 20, 60, "idle", COLOR_BAD),
        ("sprite_bad_duck", 20, 30, "duck", COLOR_BAD),
        ("sprite_bad_punch", 45, 60, "punch", COLOR_BAD),
        ("sprite_bad_kick", 45, 60, "kick", COLOR_BAD),
        ("sprite_bad_jump", 20, 60, "jump", COLOR_BAD),
        ("sprite_bad_hurt", 20, 60, "hurt", COLOR_BAD),
    ]

    for name, w, h, act, col in tasks:
        c = Canvas(w, h, BG_COLOR)
        draw_sprite(c, act, col)
        save_as_c(c, name)