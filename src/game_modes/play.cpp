const f32 LEVEL_GROWTH_RATE = 0.15f;
const f32 NEXT_LEVEL_DELAY  = 1.5f;

const u32 START_ASTEROIDS = 4;

u32 current_level;
u32 current_asteroids;

bool is_waiting_for_next_level;
f32  next_level_timer;

bool is_paused;

u32 player_lives;
u32 player_score;
u32 player_score_since_last_life;

f32 enemy_respawn_timer;

Player* the_player;
Enemy*  the_enemy;

void start_level(u32 level) {
    f32 growth = powf(1.0f + LEVEL_GROWTH_RATE, (f32) level);
    current_asteroids = (u32) floorf(START_ASTEROIDS * growth);

    current_level = level;

    for (u32 i = 0; i < current_asteroids; i++) {
        Asteroid* asteroid = create_entity(ENTITY_TYPE_ASTEROID)->asteroid;
        set_asteroid_size(asteroid, ASTEROID_SIZE_LARGE);

        u32 side = get_random_out_of(4);
        switch (side) {
            case 0: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_left, world_left + 2.5f), 
                    get_random_between(world_bottom, world_top));

                break;
            }
            case 1: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_left, world_right), 
                    get_random_between(world_top, world_top - 2.5f));

                break;
            }
            case 2: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_right, world_right - 2.5f), 
                    get_random_between(world_bottom, world_top));

                break;
            }
            case 3: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_left, world_right), 
                    get_random_between(world_bottom, world_bottom + 2.5f));

                break;
            }
            invalid_default_case();
        }
    }

    printf("Started level %u (%u asteroids)\n", current_level, current_asteroids);
}

void add_score(u32 score) {
    player_score += score;
    player_score_since_last_life += score;

    if (player_score_since_last_life >= 10000) {
        player_score_since_last_life = 0;
        player_lives += 1;
    }
}

void spawn_player() {
    the_player = create_entity(ENTITY_TYPE_PLAYER)->player;
}

void kill_player() {
    if (!the_player) return;

    if (the_player->has_shield) {
        destroy_shield(the_player);
        return;
    }

    player_lives -= 1;
    
    destroy_entity(the_player->entity);
    the_player = null;
}

void spawn_enemy() {
    the_enemy = create_entity(ENTITY_TYPE_ENEMY)->enemy;

    if (player_score >= 40000) {
        set_enemy_mode(the_enemy, ENEMY_MODE_HARD);
    }
    else {
        if (get_random_chance(4)) {
            set_enemy_mode(the_enemy, ENEMY_MODE_HARD);
        }
        else {
            set_enemy_mode(the_enemy, ENEMY_MODE_EASY);
        }
    }
}

void kill_enemy() {
    destroy_entity(the_enemy->entity);
    the_enemy = null;

    enemy_respawn_timer = get_random_between(5.0f, 15.0f);
}

void start_play() {
    is_waiting_for_next_level = false;
    is_paused = false;

    player_lives = 3;
    player_score = 0;

    enemy_respawn_timer = get_random_between(5.0f, 15.0f);

    spawn_player();
    start_level(1);
}

void stop_play() {
    // clear_entities();
}

void update_play() {
    set_projection(gui_projection);

    if (player_lives) {
        for (u32 i = 0; i < player_lives; i++) {
            f32 width  = sprite_ui_ship.width  * 1.75f;
            f32 height = sprite_ui_ship.height * 1.75f;

            Vector2 position = make_vector2(50.0f, 50.0f);

            position += make_vector2(width / 2.0f, height / 2.0f);
            position += make_vector2(i * (width + 15.0f), 0.0f);

            set_transform(make_transform_matrix(position, -45.0f));
            draw_sprite(&sprite_ui_ship, height);
        }

        utf8* score_text = format_string("%u", player_score);

        set_transform(make_transform_matrix(
            make_vector2(
                platform.window_width  - 50.0f - get_text_width(&font_nasalization, 45.0f, score_text), 
                platform.window_height - 50.0f - get_text_height(&font_nasalization, 45.0f))));

        draw_text(&font_nasalization, 45.0f, score_text);

        if (input.key_escape.down) {
            if (is_paused) {
                simulate_entities = true;
                is_paused = false;
            }
            else {
                simulate_entities = false;
                is_paused = true;
            }
        }

        if (is_paused) {
            draw_rectangle(
                make_rectangle2(make_vector2(), (f32) platform.window_width, (f32) platform.window_height), 
                make_color(0.0f, 0.0f, 0.0f, 0.25f));

            utf8* title = "Paused";
            utf8* options[] = {
                "Resume",
                "Quit"
            };

            f32 title_size   = 45.0f;
            f32 option_size  = 32.0f;
            f32 padding_size = 10.0f;

            Vector2 layout;

            {
                f32 total_height = 0.0f;

                total_height += get_text_height(&font_nasalization, title_size);
                total_height += padding_size;

                for (u32 i = 0; i < count_of(options); i++) {
                    total_height += get_text_height(&font_nasalization, option_size);
                    total_height += padding_size;
                }

                layout.x = (platform.window_width / 2.0f) - (get_text_width(&font_nasalization, title_size, title) / 2.0f);
                layout.y = (platform.window_height / 2.0f) + (total_height / 2.0f);
            }

            set_transform(make_transform_matrix(layout));
            draw_text(&font_nasalization, title_size, title);

            layout.y -= get_text_height(&font_nasalization, title_size);
            layout.y -= padding_size;

            for (u32 i = 0; i < count_of(options); i++) {
                utf8* option = options[i];

                f32 width  = get_text_width(&font_nasalization, option_size, option);
                f32 height = get_text_height(&font_nasalization, option_size);

                layout.x = (platform.window_width / 2.0f) - (width / 2.0f);

                Rectangle2 dimensions = make_rectangle2(layout, width, height);

                Vector2 world_position = unproject(
                    input.mouse_x, 
                    input.mouse_y, 
                    platform.window_width, 
                    platform.window_height, 
                    gui_projection);

                Color color = make_color(1.0f, 1.0f, 1.0f);

                if (contains(dimensions, world_position)) {
                    color = make_color(1.0f, 1.0f, 0.0f);

                    if (input.mouse_left.down) {
                        switch (i) {
                            case 0: {
                                simulate_entities = true;
                                is_paused = false;

                                break;
                            }
                            case 1: {
                                if (the_player) {
                                    destroy_entity(the_player->entity);
                                }

                                simulate_entities = true;
                                switch_game_mode(GAME_MODE_MENU);
                                
                                break;
                            }
                        }
                    }
                }

                set_transform(make_transform_matrix(layout));
                draw_text(&font_nasalization, option_size, option, color);

                layout.y -= get_text_height(&font_nasalization, option_size);
                layout.y -= padding_size;
            }
        }
        else {
            if (is_waiting_for_next_level) {
                if ((next_level_timer -= timers.delta) <= 0.0f) {
                    is_waiting_for_next_level = false;
                    start_level(current_level + 1);
                }
            }
            else {
                if (!the_enemy && (enemy_respawn_timer -= timers.delta) <= 0.0f) {
                    spawn_enemy();
                }

                if (!asteroids.count && !the_enemy) {
                    is_waiting_for_next_level = true;
                    next_level_timer = NEXT_LEVEL_DELAY;
                }
            }

             if (!the_player) {
                utf8* lives_text = format_string("You have %u lives left", player_lives);
                f32   lives_size = 30.0f;

                f32 lives_width  = get_text_width(&font_nasalization, lives_size, lives_text);
                f32 lives_height = get_text_height(&font_nasalization, lives_size);

                utf8* spawn_text = "Press space bar to spawn your ship";
                f32   spawn_size = 24.0f;

                f32 spawn_width  = get_text_width(&font_nasalization, spawn_size, spawn_text);
                f32 spawn_height = get_text_height(&font_nasalization, spawn_size);

                set_transform(make_transform_matrix(
                    make_vector2(
                        (platform.window_width  / 2.0f) - (lives_width / 2.0f), 
                        (platform.window_height / 2.0f) + 5.0f)));
                
                draw_text(&font_nasalization, lives_size, lives_text);

                set_transform(make_transform_matrix(
                    make_vector2(
                        (platform.window_width  / 2.0f) - (spawn_width / 2.0f), 
                        (platform.window_height / 2.0f) - 5.0f - spawn_height)));
                
                draw_text(&font_nasalization, spawn_size, spawn_text);

                if (input.key_space.down) {
                    spawn_player();
                }
            }
        }
    }
    else {
        utf8* score_text = format_string("You scored %u points", player_score);
        f32   score_size = 30.0f;

        f32 score_width  = get_text_width(&font_nasalization, score_size, score_text);
        f32 score_height = get_text_height(&font_nasalization, score_size);

        utf8* play_again_text = "Press space bar to continue";
        f32   play_again_size = 24.0f;

        f32 play_again_width  = get_text_width(&font_nasalization, play_again_size, play_again_text);
        f32 play_again_height = get_text_height(&font_nasalization, play_again_size);

        set_transform(make_transform_matrix(
            make_vector2(
                (platform.window_width  / 2.0f) - (score_width / 2.0f), 
                (platform.window_height / 2.0f) + 5.0f)));

        draw_text(&font_nasalization, score_size, score_text);

        set_transform(make_transform_matrix(
            make_vector2(
                (platform.window_width  / 2.0f) - (play_again_width / 2.0f), 
                (platform.window_height / 2.0f) - 5.0f - play_again_height)));

        draw_text(&font_nasalization, play_again_size, play_again_text);

        if (input.key_space.down) {
            FILE* scores_file = fopen(format_string("%s/scores.txt", get_executable_directory()), "ab");
            fprintf(scores_file, "%u, %u\n", player_score, (u32) time(null));

            printf("Wrote score to 'scores.txt'\n");
            fclose(scores_file);

            switch_game_mode(GAME_MODE_MENU);
        }
    }
}