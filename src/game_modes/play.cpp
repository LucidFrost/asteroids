const f32 LEVEL_GROWTH_RATE = 0.15f;
const f32 NEXT_LEVEL_DELAY  = 1.5f;

const u32 START_ASTEROIDS = 4;

u32 current_level;
u32 current_asteroids;

bool is_waiting_for_next_level;
f32  next_level_timer;

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
        Asteroid* asteroid = create_entity(Entity_Type::ASTEROID)->asteroid;
        set_asteroid_size(asteroid, Asteroid_Size::LARGE);

        while (1) {
            asteroid->entity->position = make_vector2(
                get_random_between(world_left, world_right), 
                get_random_between(world_bottom, world_top));

            if (!does_collide(asteroid->entity, the_player->entity)) break;
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
    the_player = create_entity(Entity_Type::PLAYER)->player;
}

void kill_player() {
    if (the_player->has_shield) {
        destroy_shield(the_player);
        return;
    }

    player_lives -= 1;
    
    destroy_entity(the_player->entity);
    the_player = null;
}

void spawn_enemy() {
    the_enemy = create_entity(Entity_Type::ENEMY)->enemy;

    if (player_score >= 40000) {
        set_enemy_mode(the_enemy, Enemy_Mode::HARD);
    }
    else {
        if (get_random_chance(4)) {
            set_enemy_mode(the_enemy, Enemy_Mode::HARD);
        }
        else {
            set_enemy_mode(the_enemy, Enemy_Mode::EASY);
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

    // if (input.key_escape.down) {
    //     switch_game_mode(Game_Mode::MENU);
    // }

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

    if (player_lives) {
        for (u32 i = 0; i < player_lives; i++) {
            f32 width  = player_life_sprite.width  * 1.75f;
            f32 height = player_life_sprite.height * 1.75f;

            Vector2 position = make_vector2(50.0f, 50.0f);

            position += make_vector2(width / 2.0f, height / 2.0f);
            position += make_vector2(i * (width + 15.0f), 0.0f);

            set_transform(make_transform_matrix(position, -45.0f));
            draw_sprite(&player_life_sprite, width, height);
        }

        utf8* score_text = format_string("%u", player_score);

        set_transform(make_transform_matrix(
            make_vector2(
                platform.window_width  - 50.0f - get_text_width(&font_nasalization, 45.0f, score_text), 
                platform.window_height - 50.0f - get_text_height(&font_nasalization, 45.0f))));

        draw_text(&font_nasalization, 45.0f, score_text);

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

            switch_game_mode(Game_Mode::MENU);
        }
    }
}