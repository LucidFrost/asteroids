// @todo: Move the enemy 'spawn' code out into game?
// despawn after killing player
// can the enemy itself collide with the player?
// increase accuracy of small ship as score increases

void spawn_enemy(Enemy* enemy) {
    assert(enemy->is_dead);

    if (the_player->score >= 40000) {
        enemy->mode = Enemy_Mode::SMALL;
    }
    else {
        if (get_random_chance(4)) {
            enemy->mode = Enemy_Mode::SMALL;
        }
        else {
            enemy->mode = Enemy_Mode::BIG;
        }
    }

    switch (enemy->mode) {
        case Enemy_Mode::SMALL: {
            enemy->entity->sprite      = &enemy_small_sprite;
            enemy->entity->sprite_size = 0.75f;

            enemy->score     = 1000;
            enemy->fire_rate = 0.5f;

            break;
        }
        case Enemy_Mode::BIG: {
            enemy->entity->sprite      = &enemy_big_sprite;
            enemy->entity->sprite_size = 1.5f;
            
            enemy->score     = 200;
            enemy->fire_rate = 1.5f;

            break;
        }
        default: {
            assert(false);
            break;
        }
    }

    if (get_random_chance(2)) {
        enemy->entity->position = make_vector2(WORLD_RIGHT, get_random_between(WORLD_BOTTOM, WORLD_TOP));
        enemy->velocity = make_vector2(-3.0f, 0.0f);
    }
    else {
        enemy->entity->position = make_vector2(WORLD_LEFT, get_random_between(WORLD_BOTTOM, WORLD_TOP));
        enemy->velocity = make_vector2(3.0f, 0.0f);
    }

    enemy->next_fire = enemy->fire_rate;
    enemy->is_dead   = false;

    enemy->entity->is_visible      = true;
    enemy->entity->has_collider    = true;
    enemy->entity->collider_radius = enemy->entity->sprite_size / 2.0f;

    play_sound(&spawn_sound);
}

void kill_enemy(Enemy* enemy) {
    assert(!enemy->is_dead);

    enemy->entity->is_visible = false;

    enemy->respawn_timer = get_random_between(5.0f, 15.0f);
    enemy->is_dead       = true;

    play_sound(&kill_01_sound);
}

void on_create(Enemy* enemy) {
    enemy->respawn_timer = get_random_between(5.0f, 15.0f);
}

void on_destroy(Enemy* enemy) {

}

void on_update(Enemy* enemy) {
    if (enemy->is_dead) {
        if ((enemy->respawn_timer -= time.delta) <= 0.0f) {
            spawn_enemy(enemy);
        }
    }
    else {
        enemy->entity->position    += enemy->velocity * time.delta;
        enemy->entity->orientation -= 100.0f * time.delta;

        if ((enemy->next_fire -= time.delta) <= 0.0f && !the_player->is_dead) {
            enemy->next_fire = enemy->fire_rate;

            f32 fire_angle = 0.0f;
            switch (enemy->mode) {
                case Enemy_Mode::SMALL: {
                    fire_angle = get_angle(normalize(the_player->entity->position - enemy->entity->position)) + get_random_between(-25.0f, 25.0f);
                    break;
                }
                case Enemy_Mode::BIG: {
                    fire_angle = get_random_between(0.0f, 360.0f);
                    break;
                }
                default: {
                    assert(false);
                    break;
                }
            }

            Laser* laser = create_entity(Entity_Type::LASER)->laser;
            laser->shooter = enemy->entity;

            laser->entity->position    = enemy->entity->position + (get_direction(fire_angle) * enemy->entity->sprite_size);
            laser->entity->orientation = fire_angle;
        }
    }
}

void on_collision(Enemy* enemy, Entity* them) {
    switch (them->type) {
        case Entity_Type::ASTEROID: {
            if (!enemy->is_dead) {
                kill_enemy(enemy);
                destroy_entity(them);
            }

            break;
        }
        case Entity_Type::PLAYER: {
            if (!enemy->is_dead && !them->player->is_dead) {
                kill_enemy(enemy);
                kill_player(them->player);
            }
            
            break;
        }
    }
}