from PIL import Image, ImageDraw

# --- Configuration ---
# Colors (R, G, B)
BG_COLOR = (0, 4, 16)       # Background (matches config.h)
SKIN_COLOR = (255, 200, 150)
GLOVE_COLOR = (255, 0, 0)   # Red gloves/shoes

# Character Themes
COLOR_GOOD = (0, 255, 0)    # Green
COLOR_BAD = (180, 0, 255)   # Purple

def create_sprite(filename, width, height, action, body_color):
    # Create blank image
    img = Image.new('RGB', (width, height), BG_COLOR)
    draw = ImageDraw.Draw(img)

    # We assume the "body" is always aligned to the LEFT side (x=0 to 20)
    
    if action == "idle":
        # Head
        draw.rectangle([6, 4, 13, 11], fill=SKIN_COLOR)
        # Torso
        draw.rectangle([5, 12, 14, 35], fill=body_color)
        # Legs
        draw.rectangle([5, 36, 8, 59], fill=body_color)
        draw.rectangle([11, 36, 14, 59], fill=body_color)
        # Arms (at sides)
        draw.rectangle([2, 12, 4, 30], fill=SKIN_COLOR)
        draw.rectangle([15, 12, 17, 30], fill=SKIN_COLOR)

    elif action == "duck":
        # Head (squatted down)
        draw.rectangle([6, 2, 13, 9], fill=SKIN_COLOR)
        # Torso (Compressed)
        draw.rectangle([5, 10, 14, 22], fill=body_color)
        # Legs (Crouched)
        draw.rectangle([4, 23, 15, 29], fill=body_color)
        # Arms (Guarding)
        draw.rectangle([15, 8, 18, 18], fill=SKIN_COLOR)

    elif action == "punch":
        # Head
        draw.rectangle([6, 4, 13, 11], fill=SKIN_COLOR)
        # Torso
        draw.rectangle([5, 12, 14, 35], fill=body_color)
        # Legs (Braced)
        draw.rectangle([5, 36, 8, 59], fill=body_color) 
        draw.rectangle([11, 36, 18, 59], fill=body_color) 
        # Back Arm
        draw.rectangle([4, 14, 6, 25], fill=SKIN_COLOR)
        # PUNCHING ARM (Extends right)
        draw.rectangle([14, 14, 25, 18], fill=SKIN_COLOR)
        draw.rectangle([25, 14, 42, 20], fill=GLOVE_COLOR)

    elif action == "kick":
        # Head
        draw.rectangle([6, 4, 13, 11], fill=SKIN_COLOR)
        # Torso (Leaning back)
        draw.rectangle([3, 12, 12, 35], fill=body_color)
        # Left Leg (Standing)
        draw.rectangle([5, 36, 9, 59], fill=body_color)
        # KICKING LEG (Extends right and up)
        draw.rectangle([12, 30, 25, 36], fill=body_color)
        draw.rectangle([25, 25, 43, 32], fill=GLOVE_COLOR)
        # Arms (Balance)
        draw.rectangle([2, 15, 4, 25], fill=SKIN_COLOR)

    elif action == "jump":
        # Head
        draw.rectangle([6, 2, 13, 9], fill=SKIN_COLOR) # Head slightly higher
        # Torso
        draw.rectangle([5, 10, 14, 33], fill=body_color)
        # Legs (Tucked up)
        draw.rectangle([4, 34, 15, 45], fill=body_color)
        # Shoes (Dangling)
        draw.rectangle([4, 45, 7, 50], fill=GLOVE_COLOR)
        draw.rectangle([12, 45, 15, 50], fill=GLOVE_COLOR)
        # Arms (Flailing up)
        draw.rectangle([1, 8, 3, 25], fill=SKIN_COLOR)
        draw.rectangle([16, 8, 18, 25], fill=SKIN_COLOR)

    elif action == "hurt":
        # Head (Knocked back)
        draw.rectangle([4, 4, 11, 11], fill=SKIN_COLOR)
        # Torso (Leaning back)
        draw.rectangle([6, 12, 15, 35], fill=body_color)
        # Legs (Staggering)
        draw.rectangle([2, 36, 6, 59], fill=body_color)
        draw.rectangle([10, 36, 14, 55], fill=body_color) # One leg lifted
        # Arms (Thrown up in shock)
        draw.rectangle([0, 10, 2, 28], fill=SKIN_COLOR)
        draw.rectangle([16, 8, 18, 26], fill=SKIN_COLOR)
        # Visual Flash (White X on chest)
        draw.line([8, 15, 13, 25], fill=(255, 255, 255), width=1)
        draw.line([13, 15, 8, 25], fill=(255, 255, 255), width=1)

    # Save
    img.save(filename)
    print(f"Saved {filename}")

if __name__ == "__main__":
    
    # --- Good Guy Sprites ---
    create_sprite("sprite_idle.png", 20, 60, "idle", COLOR_GOOD)
    create_sprite("sprite_duck.png", 20, 30, "duck", COLOR_GOOD)
    create_sprite("sprite_punch.png", 45, 60, "punch", COLOR_GOOD)
    create_sprite("sprite_kick.png", 45, 60, "kick", COLOR_GOOD)
    create_sprite("sprite_jump.png", 20, 60, "jump", COLOR_GOOD)
    create_sprite("sprite_hurt.png", 20, 60, "hurt", COLOR_GOOD)

    # --- Bad Guy Sprites ---
    create_sprite("sprite_bad_idle.png", 20, 60, "idle", COLOR_BAD)
    create_sprite("sprite_bad_duck.png", 20, 30, "duck", COLOR_BAD)
    create_sprite("sprite_bad_punch.png", 45, 60, "punch", COLOR_BAD)
    create_sprite("sprite_bad_kick.png", 45, 60, "kick", COLOR_BAD)
    create_sprite("sprite_bad_jump.png", 20, 60, "jump", COLOR_BAD)
    create_sprite("sprite_bad_hurt.png", 20, 60, "hurt", COLOR_BAD)