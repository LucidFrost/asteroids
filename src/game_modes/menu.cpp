enum Menu_Mode {
    MENU_MODE_NONE,
    MENU_MODE_MAIN,
    MENU_MODE_CUSTOMIZE,
    MENU_MODE_SCOREBOARD,
    MENU_MODE_SETTINGS
};

Menu_Mode menu_mode;

f32 menu_title_size   = 45.0f;
f32 menu_option_size  = 32.0f;
f32 menu_padding_size = 10.0f;

struct Score {
    u32 value;
    u32 time;
};

Score high_scores[10];

Array<Score> sort_scores(Array<Score> scores) {
    if (scores.count < 2) return scores;
    u32 middle = scores.count / 2;
    
    Array<Score> left  = copy(&scores, 0, middle);
    Array<Score> right = copy(&scores, middle, scores.count);

    left  = sort_scores(left);
    right = sort_scores(right);

    Array<Score> sorted;
    sorted.allocator = &temp_allocator;

    while (left.count && right.count) {
        if (left[0].value >= right[0].value) {
            add(&sorted, left[0]);
            remove(&left, 0);
        }
        else {
            add(&sorted, right[0]);
            remove(&right, 0);
        }
    }

    while (left.count) {
        add(&sorted, left[0]);
        remove(&left, 0);
    }

    while (right.count) {
        add(&sorted, right[0]);
        remove(&right, 0);
    }

    return sorted;
}

void start_menu() {
    menu_mode = MENU_MODE_MAIN;

    if (!asteroids.count) {
        for (u32 i = 0 ; i < 4; i++) {
            Asteroid* asteroid = create_entity(ENTITY_TYPE_ASTEROID)->asteroid;
            set_asteroid_size(asteroid, ASTEROID_SIZE_LARGE);

            asteroid->entity->position = make_vector2(
                get_random_between(world_left, world_right), 
                get_random_between(world_bottom, world_top));
        }
    }
}

void stop_menu() {
    for_each (Asteroid* asteroid, &asteroids) {
        destroy_entity(asteroid->entity);
    }
}

void update_menu() {
    if (input.key_escape.down) {
        menu_mode = MENU_MODE_MAIN;
    }

    switch (menu_mode) {
        case MENU_MODE_MAIN: {
            if (input.key_escape.down) {
                platform.should_quit = true;
            }

            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Asteroids!", 45.0f);
                gui_pad(10.0f);

                if (gui_button("Play", 32.0f)) {
                    switch_game_mode(GAME_MODE_PLAY);
                }

                gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));

                if (gui_button("Customize", 32.0f)) {
                    menu_mode = MENU_MODE_CUSTOMIZE;
                }

                gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));

                if (gui_button("Scoreboard", 32.0f)) {
                    menu_mode = MENU_MODE_SCOREBOARD;

                    Array<Score> scores;
                    scores.allocator = &temp_allocator;

                    FILE* scores_file = fopen(SCORES_FILE_NAME, "rb");
                    if (scores_file) {
                        while (!feof(scores_file)) {
                            Score score;
                            
                            i32 result = fscanf(scores_file, "%u, %u", &score.value, &score.time);
                            if (result <= 0) break;

                            add(&scores, score);
                        }

                        printf("Read scores from '%s'\n", SCORES_FILE_NAME);
                        fclose(scores_file);
                    }
                    else {
                        printf("Failed to read scores from '%s', the file does not exist\n", SCORES_FILE_NAME);
                    }

                    Array<Score> sorted_scores = sort_scores(scores);

                    for (u32 i = 0; i < count_of(high_scores); i++) {
                        if (i < sorted_scores.count) {
                            high_scores[i] = sorted_scores[i];
                        }
                        else {
                            high_scores[i].value = 0;
                            high_scores[i].time  = 0;
                        }
                    }
                }

                gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));
                
                if (gui_button("Settings", 32.0f)) {
                    menu_mode = MENU_MODE_SETTINGS;
                }

                gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));

                if (gui_button("Quit", 32.0f)) {
                    platform.should_quit = true;
                }
            }
            end_layout();

            break;
        }
        case MENU_MODE_CUSTOMIZE: {
            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Customize", 45.0f);
                gui_pad(10.0f);

                // @todo: Add color choice
                // @todo: Add score unlocks for customization
                // @todo: Add custom ships

                begin_layout(GUI_ADVANCE_HORIZONTAL, GUI_ANCHOR_CENTER); {
                    if (gui_button("<", 32.0f)) {
                        printf("Last ship\n");
                    }

                    gui_image(get_ship_sprite(SHIP_COLOR_RED, SHIP_TYPE_2), 150.0f);

                    if (gui_button(">", 32.0f)) {
                        printf("Next ship\n");
                    }
                }
                end_layout();

                begin_layout(GUI_ADVANCE_HORIZONTAL, GUI_ANCHOR_CENTER); {
                    if (gui_button("Back", 32.0f)) {
                        menu_mode = MENU_MODE_MAIN;
                    }

                    if (gui_button("Accept", 32.0f)) {
                        printf("Accept!\n");
                    }
                }
                end_layout();
            }
            end_layout();

            break;
        }
        case MENU_MODE_SCOREBOARD: {
            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Scoreboard", 45.0f);
                gui_pad(10.0f);

                for (u32 i = 0; i < count_of(high_scores); i++) {
                    time_t time_value = (time_t) high_scores[i].time;
                    tm* gm_time = gmtime(&time_value);

                    u32 month = gm_time->tm_mon + 1;
                    u32 day   = gm_time->tm_mday;
                    u32 year  = 1900 + gm_time->tm_year;

                    gui_text(format_string("%u) %u [%u/%u/%u]", i + 1, high_scores[i].value, month, day, year), 32.0f);
                    gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));
                }

                gui_pad(10.0f);

                begin_layout(GUI_ADVANCE_HORIZONTAL, 10.0f); {
                    if (gui_button("Back", 32.0f)) {
                        menu_mode = MENU_MODE_MAIN;
                    }

                    if (gui_button("Clear", 32.0f)) {
                        FILE* scores_file = fopen(SCORES_FILE_NAME, "wb");

                        printf("Cleared scores from '%s'\n", SCORES_FILE_NAME);
                        fclose(scores_file);

                        for (u32 i = 0; i < count_of(high_scores); i++) {
                            high_scores[i].value = 0;
                            high_scores[i].time  = 0;
                        }
                    }
                }
                end_layout();
            }
            end_layout();

            break;
        }
        case MENU_MODE_SETTINGS: {
            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Settings", 45.0f);
                gui_pad(10.0f);

                if (gui_button("Toggle Fullscreen", 32.0f)) {
                    toggle_fullscreen();

                    FILE* settings_file = fopen(SETTINGS_FILE_NAME, "wb");
                    fprintf(settings_file, "fullscreen=%s\n", platform.is_fullscreen ? "yes" : "no");

                    printf("Wrote settings to '%s'\n", SETTINGS_FILE_NAME);
                    fclose(settings_file);
                }

                gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));

                if (gui_button("Back", 32.0f)) {
                    menu_mode = MENU_MODE_MAIN;
                }
            }
            end_layout();

            break;
        }
        invalid_default_case();
    }
}