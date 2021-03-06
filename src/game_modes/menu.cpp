enum Menu_Mode {
    MENU_MODE_NONE,
    MENU_MODE_MAIN,
    MENU_MODE_CUSTOMIZE,
    MENU_MODE_SCOREBOARD,
    MENU_MODE_SETTINGS,
    MENU_MODE_CREDITS
};

Menu_Mode menu_mode;

const f32 MENU_TITLE_SIZE  = 45.0f;
const f32 MENU_OPTION_SIZE = 32.0f;

const utf8* SETTINGS_FILE_NAME = "settings.txt";
const utf8* SCORES_FILE_NAME   = "scores.txt";

Ship_Color ship_color;
Ship_Type  ship_type;

void load_settings() {
    FILE* settings_file = fopen(SETTINGS_FILE_NAME, "rb");
    if (settings_file) {
        utf8 fullscreen[4];
        fscanf(settings_file, "fullscreen=%s\n", fullscreen);

        if (compare(fullscreen, "no")) {
            toggle_fullscreen();
        }

        utf8 sound[4];
        fscanf(settings_file, "sound=%s\n", sound);

        if (compare(sound, "no")) {
            toggle_sound();
        }

        u32 read_ship_color;
        fscanf(settings_file, "ship_color=%u\n", &read_ship_color);

        if (0 <= read_ship_color && read_ship_color < SHIP_COLOR_COUNT) {
            ship_color = (Ship_Color) read_ship_color;
        }

        u32 read_ship_type;
        fscanf(settings_file, "ship_type=%u\n", &read_ship_type);

        if (0 <= read_ship_type && read_ship_type < SHIP_TYPE_COUNT) {
            ship_type = (Ship_Type) read_ship_type;
        }

        printf("Read settings from '%s'\n", SETTINGS_FILE_NAME);
        fclose(settings_file);
    }
    else {
        printf("Failed to read settings from '%s', the file does not exist\n", SETTINGS_FILE_NAME);
    }
}

void save_settings() {
    FILE* settings_file = fopen(SETTINGS_FILE_NAME, "wb");
    
    fprintf(settings_file, "fullscreen=%s\n", platform.is_fullscreen ? "yes" : "no");
    fprintf(settings_file, "sound=%s\n", sound_is_on ? "yes" : "no");
    fprintf(settings_file, "ship_color=%u\n", (u32) ship_color);
    fprintf(settings_file, "ship_type=%u\n", (u32) ship_type);

    printf("Wrote settings to '%s'\n", SETTINGS_FILE_NAME);
    fclose(settings_file);
}

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
    switch (menu_mode) {
        case MENU_MODE_MAIN: {
            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Asteroids!", MENU_TITLE_SIZE);
                gui_pad(10.0f);

                if (gui_button("Play", MENU_OPTION_SIZE)) {
                    switch_game_mode(GAME_MODE_SURVIVAL);
                }

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));

                if (gui_button("Customize", MENU_OPTION_SIZE)) {
                    menu_mode = MENU_MODE_CUSTOMIZE;
                }

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));

                if (gui_button("Scoreboard", MENU_OPTION_SIZE)) {
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

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));
                
                if (gui_button("Settings", MENU_OPTION_SIZE)) {
                    menu_mode = MENU_MODE_SETTINGS;
                }

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));

                if (gui_button("Credits", MENU_OPTION_SIZE)) {
                    menu_mode = MENU_MODE_CREDITS;
                }

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));

                if (gui_button("Quit", MENU_OPTION_SIZE)) {
                    platform.should_quit = true;
                }
            }
            end_layout();

            break;
        }
        case MENU_MODE_CUSTOMIZE: {
            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Customize", MENU_TITLE_SIZE);
                gui_pad(10.0f);

                begin_layout(GUI_ADVANCE_HORIZONTAL); {
                    begin_layout(GUI_ADVANCE_VERTICAL); {
                        begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                            gui_text("Ship Color", MENU_OPTION_SIZE);

                            begin_layout(GUI_ADVANCE_HORIZONTAL, 10.0f); {
                                if (gui_button(to_u32(&ship_color), "<", MENU_OPTION_SIZE)) {
                                    if (ship_color == SHIP_COLOR_RED) {
                                        ship_color = SHIP_COLOR_ORANGE;
                                    }
                                    else {
                                        ship_color = (Ship_Color) ((u32) ship_color - 1);
                                    }

                                    save_settings();
                                }
                                
                                gui_text(to_string(ship_color), MENU_OPTION_SIZE);

                                if (gui_button(to_u32(&ship_color), ">", MENU_OPTION_SIZE)) {
                                    if (ship_color == SHIP_COLOR_ORANGE) {
                                        ship_color = SHIP_COLOR_RED;
                                    }
                                    else {
                                        ship_color = (Ship_Color) ((u32) ship_color + 1);
                                    }

                                    save_settings();
                                }
                            }
                            end_layout();

                            gui_text("Ship Type", MENU_OPTION_SIZE);

                            begin_layout(GUI_ADVANCE_HORIZONTAL, 10.0f); {
                                if (gui_button(to_u32(&ship_type), "<", MENU_OPTION_SIZE)) {
                                    if (ship_type == SHIP_TYPE_1) {
                                        ship_type = SHIP_TYPE_3;
                                    }
                                    else {
                                        ship_type = (Ship_Type) ((u32) ship_type - 1);
                                    }

                                    save_settings();
                                }

                                gui_text(to_string(ship_type), MENU_OPTION_SIZE);

                                if (gui_button(to_u32(&ship_type), ">", MENU_OPTION_SIZE)) {
                                    if (ship_type == SHIP_TYPE_3) {
                                        ship_type = SHIP_TYPE_1;
                                    }
                                    else {
                                        ship_type = (Ship_Type) ((u32) ship_type + 1);
                                    }

                                    save_settings();
                                }
                            }
                            end_layout();
                        }
                        end_layout();
                    }
                    end_layout();

                    gui_pad(30.0f);

                    begin_layout(GUI_ADVANCE_VERTICAL); {
                        begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                            gui_pad(20.0f);
                            gui_image(get_ship_sprite(ship_type, ship_color), 100.0f);
                        }
                        end_layout();
                    }
                    end_layout();
                }
                end_layout();

                gui_pad(10.0f);

                begin_layout(GUI_ADVANCE_HORIZONTAL, 10.0f); {
                    if (gui_button("Back", MENU_OPTION_SIZE)) {
                        menu_mode = MENU_MODE_MAIN;
                    }

                    // if (gui_button("Accept", MENU_OPTION_SIZE)) {
                    //     printf("Accept!\n");
                    // }
                }
                end_layout();
            }
            end_layout();

            break;
        }
        case MENU_MODE_SCOREBOARD: {
            begin_layout(GUI_ADVANCE_VERTICAL, 10.0f, GUI_ANCHOR_CENTER); {
                gui_text("Scoreboard", MENU_TITLE_SIZE);

                for (u32 i = 0; i < count_of(high_scores); i++) {
                    if (!high_scores[i].value) continue;

                    time_t time_value = (time_t) high_scores[i].time;
                    tm* gm_time = gmtime(&time_value);

                    u32 month = gm_time->tm_mon + 1;
                    u32 day   = gm_time->tm_mday;
                    u32 year  = 1900 + gm_time->tm_year;

                    begin_layout(GUI_ADVANCE_HORIZONTAL, 25.0f); {
                        gui_text(format_string("%u)", i + 1), MENU_OPTION_SIZE);
                        gui_text(format_string("%u", high_scores[i].value), MENU_OPTION_SIZE);
                        gui_text(format_string("%u/%u/%u", month, day, year), MENU_OPTION_SIZE);
                    }
                    end_layout();
                }

                begin_layout(GUI_ADVANCE_HORIZONTAL, 10.0f); {
                    if (gui_button("Back", MENU_OPTION_SIZE)) {
                        menu_mode = MENU_MODE_MAIN;
                    }

                    if (gui_button("Clear", MENU_OPTION_SIZE)) {
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
                gui_text("Settings", MENU_TITLE_SIZE);
                gui_pad(10.0f);

                if (gui_button("Fullscreen", MENU_OPTION_SIZE)) {
                    toggle_fullscreen();
                    save_settings();
                }

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));
                
                if (gui_button("Sound", MENU_OPTION_SIZE)) {
                    toggle_sound();
                    save_settings();
                }

                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));

                if (gui_button("Back", MENU_OPTION_SIZE)) {
                    menu_mode = MENU_MODE_MAIN;
                }
            }
            end_layout();

            break;
        }
        case MENU_MODE_CREDITS: {
            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Credits", MENU_TITLE_SIZE);
                gui_pad(10.0f);

                gui_text("Art created by Kenney Vleugels (www.kenney.nl)", MENU_OPTION_SIZE);
                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));
                
                gui_text("Sound created by Kalen Reed using www.beepbox.com", MENU_OPTION_SIZE);
                gui_pad(get_font_line_gap(gui_context.default_font, MENU_OPTION_SIZE));
                
                gui_text("Programming and design done by Ryan Flaherty", MENU_OPTION_SIZE);
                gui_pad(10.0f);

                if (gui_button("Back", MENU_OPTION_SIZE)) {
                    menu_mode = MENU_MODE_MAIN;
                }
            }
            end_layout();

            break;
        }
        invalid_default_case();
    }
}