#include <stdint.h>

#define count_of(array) (sizeof(array) / sizeof(array[0]))

int get_string_length(char* string) {
    char* at = string;
    while (*at++);

    return (int) (at - string - 1);
}

#include "platform.cpp"
#include "math.cpp"
#include "random.cpp"
#include "draw.cpp"
#include "sound.cpp"

enum Game_Mode {
    GM_NONE,
    GM_MENU,
    GM_ASTEROIDS
};

Game_Mode game_mode          = GM_NONE;
bool      game_mode_switched = false;

void switch_game_mode(Game_Mode new_game_mode) {
    game_mode          = new_game_mode;
    game_mode_switched = true;
}

#include "gm_menu.cpp"
#include "gm_asteroids.cpp"

int main() {
    init_platform();
    init_draw();
    init_sound();

    init_menu();
    init_asteroids();

    Sprite background_sprite = load_sprite("data/sprites/background.png");

    uint64_t last_time = get_ticks();
    seed_random(last_time);

    while (is_running) {
        uint64_t start_time = get_ticks();
        
        delta_time = (double) (start_time - last_time) / (double) tick_frequency;
        last_time  = start_time;

        update_platform();
        draw_begin();

        float background_x = -HALF_VIEWPORT_WIDTH;
        float background_y = -HALF_VIEWPORT_HEIGHT;

        float background_width  = get_sprite_draw_width(&background_sprite);
        float background_height = get_sprite_draw_height(&background_sprite);

        float background_x_count = VIEWPORT_WIDTH  / background_width;
        float background_y_count = VIEWPORT_HEIGHT / background_height;

        for (int x = 0; x < background_x_count; x++) {
            for (int y = 0; y < background_y_count; y++) {
                Vector2 position = make_vector2(background_x, background_y);
                
                position.x += (x * background_width)  + (background_width  / 2.0f);
                position.y += (y * background_height) + (background_height / 2.0f);

                draw_sprite(&background_sprite, position, 0.0f);
            }
        }

        if (game_mode_switched) {
            switch (game_mode) {
                case GM_MENU: {
                    start_menu();
                    break;
                }
                case GM_ASTEROIDS: {
                    start_asteroids();
                    break;
                }
            }

            game_mode_switched = false;
        }

        switch (game_mode) {
            case GM_MENU: {
                update_menu();
                break;
            }
            case GM_ASTEROIDS: {
                update_asteroids();
                break;
            }
            default: {
                print("Unhandled game mode specified, going to menu...\n");
                switch_game_mode(GM_MENU);
                
                break;
            }
        }

        draw_text(format_string("%f", delta_time), make_vector2(16.0f, SCREEN_HEIGHT - 32.0f));
        draw_end();
    }

    return 0;
}