idf_component_register(SRCS main.c game.c movement.c attack.c sprite_renderer.c uart.c
                            sprite_idle.c sprite_duck.c sprite_punch.c sprite_kick.c sprite_jump.c sprite_hurt.c
                            sprite_bad_idle.c sprite_bad_duck.c sprite_bad_punch.c sprite_bad_kick.c sprite_bad_jump.c sprite_bad_hurt.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES esp_timer net config lcd pin joy sound cursor driver)
# PRIV_REQUIRES ...
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
