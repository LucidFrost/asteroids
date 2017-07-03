void kill_enemy();

void set_enemy_mode(Enemy* enemy, Enemy_Mode enemy_mode) {
    enemy->mode = enemy_mode;

    switch (enemy->mode) {
        case Enemy_Mode::EASY: {
            enemy->entity->sprite      = &enemy_big_sprite;
            enemy->entity->sprite_size = 1.5f;
            
            enemy->score     = 200;
            enemy->fire_rate = 1.5f;

            break;
        }
        case Enemy_Mode::HARD: {
            enemy->entity->sprite      = &enemy_small_sprite;
            enemy->entity->sprite_size = 0.75f;

            enemy->score     = 1000;
            enemy->fire_rate = 0.5f;

            break;
        }
        default: {
            assert(false);
            break;
        }
    }
    
    enemy->entity->collider_radius = enemy->entity->sprite_size / 2.0f;
}

void on_create(Enemy* enemy) {
    enemy->next_fire = enemy->fire_rate;

    enemy->entity->is_visible      = true;
    enemy->entity->has_collider    = true;

    if (get_random_chance(2)) {
        enemy->entity->position = make_vector2(world_right, get_random_between(world_bottom, world_top));
        enemy->velocity = make_vector2(-3.0f, 0.0f);
    }
    else {
        enemy->entity->position = make_vector2(world_left, get_random_between(world_bottom, world_top));
        enemy->velocity = make_vector2(3.0f, 0.0f);
    }

    play_sound(&spawn_sound);
}

void on_destroy(Enemy* enemy) {
    play_sound(&kill_01_sound);
}

void on_update(Enemy* enemy) {
    enemy->entity->position    += enemy->velocity * timers.delta;
    enemy->entity->orientation -= 100.0f * timers.delta;

    Player* player = null;
    for_each (Player* p, &players) {
        player = p;
        break;
    }

    if (player && (enemy->next_fire -= timers.delta) <= 0.0f) {
        enemy->next_fire = enemy->fire_rate;

        f32 fire_angle = 0.0f;
        switch (enemy->mode) {
            case Enemy_Mode::EASY: {
                fire_angle = get_random_between(0.0f, 360.0f);
                break;
            }
            case Enemy_Mode::HARD: {
                fire_angle = get_angle(normalize(player->entity->position - enemy->entity->position)) + get_random_between(-25.0f, 25.0f);
                break;
            }
            default: {
                assert(false);
                break;
            }
        }

        Laser* laser = create_entity(Entity_Type::LASER)->laser;

        laser->shooter_id          = enemy->entity->id;
        laser->entity->sprite      = &laser_red_sprite;
        laser->entity->position    = enemy->entity->position + (get_direction(fire_angle) * enemy->entity->sprite_size);
        laser->entity->orientation = fire_angle;
    }
}

void on_collision(Enemy* enemy, Entity* them) {
    switch (them->type) {
        case Entity_Type::ASTEROID: {
            kill_enemy();

            spawn_children(them->asteroid);
            destroy_entity(them);

            break;
        }
        case Entity_Type::PLAYER: {
            kill_player();
            kill_enemy();

            break;
        }
    }
}