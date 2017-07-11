#if AS_HEADER

enum Enemy_Mode {
    ENEMY_MODE_NONE,
    ENEMY_MODE_EASY,
    ENEMY_MODE_HARD
};

struct Enemy {
    Entity* entity = null;

    Enemy_Mode mode = ENEMY_MODE_NONE;
    u32 score = 0;

    f32 fire_rate = 0.0f;
    f32 next_fire = 0.0f;

    Vector2 velocity;
};

void on_create(Enemy* enemy);
void on_destroy(Enemy* enemy);
void on_update(Enemy* enemy);
void on_collision(Enemy* enemy, Entity* them);

void kill_enemy();

#else

void set_enemy_mode(Enemy* enemy, Enemy_Mode enemy_mode) {
    enemy->mode = enemy_mode;

    switch (enemy->mode) {
        case ENEMY_MODE_EASY: {
            set_sprite(enemy->entity, get_enemy_sprite(ENEMY_COLOR_YELLOW), 1.5f);
            set_collider(enemy->entity, 0.75f);
            
            enemy->score     = 200;
            enemy->fire_rate = 0.75f;

            break;
        }
        case ENEMY_MODE_HARD: {
            set_sprite(enemy->entity, get_enemy_sprite(ENEMY_COLOR_ORANGE), 0.75f);
            set_collider(enemy->entity, 0.375f);

            enemy->score     = 1000;
            enemy->fire_rate = 0.25f;

            break;
        }
        default: {
            assert(false);
            break;
        }
    }
    
    enemy->next_fire = enemy->fire_rate;
}

void on_create(Enemy* enemy) {
    enemy->entity->position = make_vector2(world_left, get_random_between(world_bottom, world_top));
    enemy->velocity = make_vector2(get_random_chance(2) ? -3.0f : 3.0f, 0.0f);

    play_sound(&sound_spawn);
}

void on_destroy(Enemy* enemy) {
    play_sound(get_kill_sound());
}

void on_update(Enemy* enemy) {
    enemy->entity->position    += enemy->velocity * timers.delta;
    enemy->entity->orientation -= 100.0f * timers.delta;

    Player* player = null;
    for_each (Player* p, &players) {
        player = p;
        break;
    }

    if (player) {
        if ((enemy->next_fire -= timers.delta) <= 0.0f) {
            enemy->next_fire = enemy->fire_rate;

            f32 fire_angle = 0.0f;
            switch (enemy->mode) {
                case ENEMY_MODE_EASY: {
                    fire_angle = get_random_between(0.0f, 360.0f);
                    break;
                }
                case ENEMY_MODE_HARD: {
                    fire_angle = get_angle(
                        normalize(player->entity->position - enemy->entity->position)) + 
                        get_random_between(-15.0f, 15.0f);

                    break;
                }
                invalid_default_case();
            }

            Laser* laser = create_entity(ENTITY_TYPE_LASER)->laser;
            init_laser(laser, LASER_COLOR_RED, enemy->entity, fire_angle);
            
            play_sound(get_enemy_fire_sound());
        }
    }
    else {
        enemy->next_fire = enemy->fire_rate;
    }
}

void on_collision(Enemy* enemy, Entity* them) {
    switch (them->type) {
        case ENTITY_TYPE_ASTEROID: {
            kill_enemy();

            spawn_children(them->asteroid);
            destroy_entity(them);

            break;
        }
        case ENTITY_TYPE_PLAYER: {
            damage_player(them->player);
            kill_enemy();

            break;
        }
    }
}

#endif