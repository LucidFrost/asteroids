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
                    get_random_between(world_left, world_left + (world_width / 4.0f)), 
                    get_random_between(world_bottom, world_top));

                break;
            }
            case 1: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_left, world_right), 
                    get_random_between(world_top, world_top - (world_width / 4.0f)));

                break;
            }
            case 2: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_right, world_right - (world_width / 4.0f)), 
                    get_random_between(world_bottom, world_top));

                break;
            }
            case 3: {
                asteroid->entity->position = make_vector2(
                    get_random_between(world_left, world_right), 
                    get_random_between(world_bottom, world_bottom + (world_width / 4.0f)));

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
    init_player(the_player, ship_type, ship_color);
}

void kill_player() {
    if (!the_player) return;

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

void start_survival() {
    is_waiting_for_next_level = false;
    is_paused = false;

    player_lives = 3;
    player_score = 0;
    player_score_since_last_life = 0;

    enemy_respawn_timer = get_random_between(5.0f, 15.0f);

    start_level(1);
    spawn_player();
}

void stop_survival() {
    
}

void update_survival() {
    // @todo: Pause button on the GUI
    // @todo: Notify when they unlock something or get another life

    if (player_lives) {
        if (input.key_escape.down) {
            if (is_paused) {
                should_simulate = true;
                is_paused = false;
            }
            else {
                should_simulate = false;
                is_paused = true;
            }
        }

        begin_layout(GUI_ADVANCE_HORIZONTAL, 15.0f, GUI_ANCHOR_BOTTOM_LEFT, 50.0f, 50.0f); {
            for (u32 i = 0; i < player_lives; i++) {
                gui_image(&sprite_ui_ship, sprite_ui_ship.height * 1.75f);
            }
        }
        end_layout();

        begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_TOP_RIGHT, 50.0f, 50.0f); {
            gui_text(format_string("%u", player_score), 45.0f);
        }
        end_layout();

        if (is_paused) {
            gui_fill(make_color(0.0f, 0.0f, 0.0f, 0.25f));

            begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                gui_text("Paused", 45.0f);
                gui_pad(10.0f);

                if (gui_button("Resume", 32.0f)) {
                    should_simulate = true;
                    is_paused = false;
                }

                gui_pad(get_font_line_gap(gui_context.default_font, 32.0f));

                if (gui_button("Quit", 32.0f)) {
                    if (the_player) {
                        destroy_entity(the_player->entity);
                    }

                    should_simulate = true;
                    switch_game_mode(GAME_MODE_MENU);
                }
            }
            end_layout();
        }
        else {
            if (is_waiting_for_next_level) {
                if ((next_level_timer -= timers.delta) <= 0.0f) {
                    is_waiting_for_next_level = false;
                    start_level(player_score > 40000 ? current_level : current_level + 1);
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
                begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
                    gui_text(format_string("You have %u lives left", player_lives), 45.0f);
                    gui_pad(10.0f);

                    if (gui_button("Respawn", 32.0f)) {
                        spawn_player();
                    }
                }
                end_layout();
            }
        }
    }
    else {
        begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_CENTER); {
            gui_text(format_string("You scored %u points", player_score), 45.0f);
            gui_pad(10.0f);

            if (gui_button("Continue", 32.0f)) {
                FILE* scores_file = fopen(SCORES_FILE_NAME, "ab");
                fprintf(scores_file, "%u, %u\n", player_score, (u32) time(null));

                printf("Wrote score to '%s'\n", SCORES_FILE_NAME);
                fclose(scores_file);

                switch_game_mode(GAME_MODE_MENU);
            }
        }
        end_layout();
    }
}