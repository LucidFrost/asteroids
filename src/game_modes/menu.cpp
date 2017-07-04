enum Menu_Mode {
    MENU_MODE_NONE,
    MENU_MODE_MAIN,
    MENU_MODE_SHIP,
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
    set_projection(gui_projection);

    if (input.key_escape.down) {
        menu_mode = MENU_MODE_MAIN;
    }

    switch (menu_mode) {
        case MENU_MODE_MAIN: {
            if (input.key_escape.down) {
                platform.should_quit = true;
            }

            utf8* title_text = "Asteroids!";

            utf8* menu_options[] = {
                "Play",
                "Ship",
                "Scoreboard",
                "Settings",
                "Quit"
            };

            f32 total_height = 0.0f;
            
            total_height += get_text_height(&font_nasalization, menu_title_size);
            total_height += menu_padding_size;

            for (u32 i = 0; i < count_of(menu_options); i++) {
                total_height += get_text_height(&font_nasalization, menu_option_size);
                total_height += menu_padding_size;
            }

            Vector2 layout = make_vector2(
                (platform.window_width  / 2.0f) - (get_text_width(&font_nasalization, menu_title_size, title_text) / 2.0f), 
                (platform.window_height / 2.0f) + (total_height / 2.0f));

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, menu_title_size, title_text);

            layout.y -= get_text_height(&font_nasalization, menu_title_size);
            layout.y -= menu_padding_size;

            for (u32 i = 0; i < count_of(menu_options); i++) {
                utf8* menu_option = menu_options[i];

                f32 width  = get_text_width(&font_nasalization, menu_option_size, menu_option);
                f32 height = get_text_height(&font_nasalization, menu_option_size);

                layout.x = (platform.window_width / 2.0f) - (width / 2.0f);

                Rectangle2 dimensions = make_rectangle2(layout, width, height);
                Vector2 world_position = unproject(input.mouse_x, input.mouse_y, platform.window_width, platform.window_height, gui_projection);

                Color color = make_color(1.0f, 1.0f, 1.0f);
                if (contains(dimensions, world_position)) {
                    color = make_color(1.0f, 1.0f, 0.0f);

                    if (input.mouse_left.down) {
                        switch (i) {
                            case 0: {
                                switch_game_mode(GAME_MODE_PLAY);
                                break;
                            }
                            case 1: {
                                menu_mode = MENU_MODE_SHIP;
                                break;
                            }
                            case 2: {
                                menu_mode = MENU_MODE_SCOREBOARD;

                                Array<Score> scores;
                                scores.allocator = &temp_allocator;

                                FILE* scores_file = fopen(format_string("%s/scores.txt", get_executable_directory()), "rb");
                                if (scores_file) {
                                    while (!feof(scores_file)) {
                                        Score score;
                                        
                                        i32 result = fscanf(scores_file, "%u, %u", &score.value, &score.time);
                                        if (result <= 0) break;

                                        add(&scores, score);
                                    }

                                    printf("Read scores from 'scores.txt'\n");
                                    fclose(scores_file);
                                }
                                else {
                                    printf("Failed to read scores from 'scores.txt', the file does not exist\n");
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

                                break;
                            }
                            case 3: {
                                menu_mode = MENU_MODE_SETTINGS;
                                break;
                            }
                            case 4: {
                                platform.should_quit = true;
                                break;
                            }
                        }
                    }
                }

                set_transform(make_transform_matrix(layout));
                draw_text(&font_nasalization, menu_option_size, menu_option, color);

                layout.y -= height;
                layout.y -= menu_padding_size;
            }

            break;
        }
        case MENU_MODE_SHIP: {
            utf8* title_text = "Ship";
            
            utf8* actions[] = {
                "Back",
                "Accept"
            };

            f32 total_height = 0.0f;

            total_height += get_text_height(&font_nasalization, menu_title_size);
            total_height += menu_padding_size;

            total_height += 200.0f;

            total_height += get_text_height(&font_nasalization, menu_option_size);
            total_height += menu_padding_size;

            Vector2 layout = make_vector2(
                (platform.window_width  / 2.0f) - (get_text_width(&font_nasalization, menu_title_size, title_text) / 2.0f), 
                (platform.window_height / 2.0f) + (total_height / 2.0f));

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, menu_title_size, title_text);

            layout.x = platform.window_width  / 2.0f;
            layout.y = platform.window_height / 2.0f;

            set_transform(make_transform_matrix(layout));
            draw_sprite(get_ship_sprite(SHIP_COLOR_RED, SHIP_TYPE_2), 150.0f);

            layout.x -= 150.0f;

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, menu_title_size, "<");

            layout.x += 300.0f;

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, menu_title_size, ">");

            layout.y -= 150.0f;

            f32 total_width = 0.0f;
            for (u32 i = 0; i < count_of(actions); i++) {
                utf8* action = actions[i];
                
                total_width += get_text_width(&font_nasalization, menu_option_size, action);
                total_width += menu_padding_size * 2.0f;
            }

            layout.x = (platform.window_width / 2.0f) - (total_width / 2.0f);

            for (u32 i = 0; i < count_of(actions); i++) {
                utf8* action = actions[i];

                f32 width  = get_text_width(&font_nasalization, menu_option_size, action);
                f32 height = get_text_height(&font_nasalization, menu_option_size);

                Rectangle2 dimensions = make_rectangle2(layout, width, height);
                Vector2 world_position = unproject(input.mouse_x, input.mouse_y, platform.window_width, platform.window_height, gui_projection);

                Color color = make_color(1.0f, 1.0f, 1.0f);
                if (contains(dimensions, world_position)) {
                    color = make_color(1.0f, 1.0f, 0.0f);

                    if (input.mouse_left.down) {
                        switch (i) {
                            case 0: {
                                menu_mode = MENU_MODE_MAIN;
                                break;
                            }
                            case 1: {
                                break;
                            }
                        }
                    }
                }

                set_transform(make_transform_matrix(layout));
                draw_text(&font_nasalization, menu_option_size, action, color);

                layout.x += width;
                layout.x += menu_padding_size * 2.0f;
            }

            break;
        }
        case MENU_MODE_SCOREBOARD: {
            utf8* title_text = "Scoreboard";
            
            utf8* actions[] = {
                "Back",
                "Clear"
            };

            f32 total_height = 0.0f;

            total_height += get_text_height(&font_nasalization, menu_title_size);
            total_height += menu_padding_size;

            for (u32 i = 0; i < count_of(high_scores); i++) {
                total_height += get_text_height(&font_nasalization, menu_option_size);
                total_height += menu_padding_size;
            }

            total_height += get_text_height(&font_nasalization, menu_option_size);
            total_height += menu_padding_size;

            Vector2 layout = make_vector2(
                (platform.window_width  / 2.0f) - (get_text_width(&font_nasalization, menu_title_size, title_text) / 2.0f), 
                (platform.window_height / 2.0f) + (total_height / 2.0f));

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, menu_title_size, title_text);

            layout.y -= get_text_height(&font_nasalization, menu_title_size);
            layout.y -= menu_padding_size;

            for (u32 i = 0; i < count_of(high_scores); i++) {
                if (high_scores[i].value) {
                    layout.x = (platform.window_width / 2.0f) - 200.0f;

                    set_transform(make_transform_matrix(layout));
                    draw_text(&font_nasalization, menu_option_size, format_string("%u.", i + 1));

                    utf8* text = format_string("%u", high_scores[i].value);

                    f32 width = get_text_width(&font_nasalization, menu_option_size, text);
                    layout.x = (platform.window_width / 2.0f) - (width / 2.0f);

                    set_transform(make_transform_matrix(layout));
                    draw_text(&font_nasalization, menu_option_size, text);

                    time_t time_value = (time_t) high_scores[i].time;
                    tm* gm_time = gmtime(&time_value);

                    u32 month = gm_time->tm_mon + 1;
                    u32 day   = gm_time->tm_mday;
                    u32 year  = 1900 + gm_time->tm_year;

                    text = format_string("%u/%u/%u", month, day, year);

                    layout.x = (platform.window_width / 2.0f) + 150.0f;

                    set_transform(make_transform_matrix(layout));
                    draw_text(&font_nasalization, menu_option_size, text);                    
                }

                layout.y -= get_text_height(&font_nasalization, menu_option_size);
                layout.y -= menu_padding_size;
            }

            f32 total_width = 0.0f;
            for (u32 i = 0; i < count_of(actions); i++) {
                utf8* action = actions[i];
                
                total_width += get_text_width(&font_nasalization, menu_option_size, action);
                total_width += menu_padding_size * 2.0f;
            }

            layout.x = (platform.window_width / 2.0f) - (total_width / 2.0f);

            for (u32 i = 0; i < count_of(actions); i++) {
                utf8* action = actions[i];

                f32 width  = get_text_width(&font_nasalization, menu_option_size, action);
                f32 height = get_text_height(&font_nasalization, menu_option_size);

                Rectangle2 dimensions = make_rectangle2(layout, width, height);
                Vector2 world_position = unproject(input.mouse_x, input.mouse_y, platform.window_width, platform.window_height, gui_projection);

                Color color = make_color(1.0f, 1.0f, 1.0f);
                if (contains(dimensions, world_position)) {
                    color = make_color(1.0f, 1.0f, 0.0f);

                    if (input.mouse_left.down) {
                        switch (i) {
                            case 0: {
                                menu_mode = MENU_MODE_MAIN;
                                break;
                            }
                            case 1: {
                                FILE* scores_file = fopen(format_string("%s/scores.txt", get_executable_directory()), "wb");

                                printf("Cleared scores from 'scores.txt'\n");
                                fclose(scores_file);

                                for (u32 i = 0; i < count_of(high_scores); i++) {
                                    high_scores[i].value = 0;
                                    high_scores[i].time  = 0;
                                }
                                
                                break;
                            }
                        }
                    }
                }

                set_transform(make_transform_matrix(layout));
                draw_text(&font_nasalization, menu_option_size, action, color);

                layout.x += width;
                layout.x += menu_padding_size * 2.0f;
            }

            break;
        }
        case MENU_MODE_SETTINGS: {
            utf8* title_text = "Settings";

            utf8* menu_options[] = {
                "Fullscreen",
                "Back"
            };

            f32 total_height = 0.0f;

            total_height += get_text_height(&font_nasalization, menu_title_size);
            total_height += menu_padding_size;

            for (u32 i = 0; i < count_of(menu_options); i++) {
                total_height += get_text_height(&font_nasalization, menu_option_size);
                total_height += menu_padding_size;
            }

            Vector2 layout = make_vector2(
                (platform.window_width  / 2.0f) - (get_text_width(&font_nasalization, menu_title_size, title_text) / 2.0f), 
                (platform.window_height / 2.0f) + (total_height / 2.0f));

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, menu_title_size, title_text);

            layout.y -= get_text_height(&font_nasalization, menu_title_size);
            layout.y -= menu_padding_size;

            for (u32 i = 0; i < count_of(menu_options); i++) {
                utf8* menu_option = menu_options[i];

                f32 width  = get_text_width(&font_nasalization, menu_option_size, menu_option);
                f32 height = get_text_height(&font_nasalization, menu_option_size);

                layout.x = (platform.window_width / 2.0f) - (width / 2.0f);

                Rectangle2 dimensions = make_rectangle2(layout, width, height);
                Vector2 world_position = unproject(input.mouse_x, input.mouse_y, platform.window_width, platform.window_height, gui_projection);

                Color color = make_color(1.0f, 1.0f, 1.0f);
                if (contains(dimensions, world_position)) {
                    color = make_color(1.0f, 1.0f, 0.0f);

                    if (input.mouse_left.down) {
                        switch (i) {
                            case 0: {
                                toggle_fullscreen();

                                FILE* settings_file = fopen(format_string("%s/settings.txt", get_executable_directory()), "wb");
                                fprintf(settings_file, "fullscreen=%s\n", platform.is_fullscreen ? "yes" : "no");

                                printf("Wrote settings to 'settings.txt'\n");
                                fclose(settings_file);

                                break;
                            }
                            case 1: {
                                menu_mode = MENU_MODE_MAIN;
                                break;
                            }
                        }
                    }
                }

                set_transform(make_transform_matrix(layout));
                draw_text(&font_nasalization, menu_option_size, menu_option, color);

                layout.y -= height;
                layout.y -= menu_padding_size;
            }

            break;
        }
        invalid_default_case();
    }
}