// @todo: Move the enemy 'spawn' code out into game?
// despawn after killing player
// can the enemy itself collide with the player?
// increase accuracy of small ship as score increases

void set_shooter(Entity* laser, Entity* shooter);

enum struct Enemy_Mode {
    NONE,
    BIG,
    SMALL
};

struct Enemy {
    Enemy_Mode mode = Enemy_Mode::NONE;
    int score = 0;

    float respawn_timer = 0.0f;
    bool is_dead = true;

    float fire_rate = 0.0f;
    float next_fire = 0.0f;

    Vector2 velocity;
};

void spawn_enemy(Entity* entity) {
    entity->enemy->is_dead = false;

    if (the_player->player->score >= 40000) {
        entity->enemy->mode = Enemy_Mode::SMALL;
    }
    else {
        if (get_random_chance(4)) {
            entity->enemy->mode = Enemy_Mode::SMALL;
        }
        else {
            entity->enemy->mode = Enemy_Mode::BIG;
        }
    }

    switch (entity->enemy->mode) {
        case Enemy_Mode::SMALL: {
            entity->collider_radius = 0.375f;

            entity->sprite      = &enemy_small_sprite;
            entity->sprite_size = 0.75f;
            entity->is_visible  = true;

            entity->enemy->score = 1000;
            entity->enemy->fire_rate = 0.5f;

            break;
        }
        case Enemy_Mode::BIG: {
            entity->collider_radius = 0.75f;
            
            entity->sprite      = &enemy_big_sprite;
            entity->sprite_size = 1.5f;
            entity->is_visible  = true;

            entity->enemy->score = 200;
            entity->enemy->fire_rate = 1.5f;

            break;
        }
        default: {
            assert(false);
            break;
        }
    }

    if (get_random_chance(2)) {
        entity->position = make_vector2(WORLD_RIGHT, get_random_between(WORLD_BOTTOM, WORLD_TOP));
        entity->enemy->velocity = make_vector2(-3.0f, 0.0f);
    }
    else {
        entity->position = make_vector2(WORLD_LEFT, get_random_between(WORLD_BOTTOM, WORLD_TOP));
        entity->enemy->velocity = make_vector2(3.0f, 0.0f);
    }

    entity->enemy->next_fire = entity->enemy->fire_rate;
}

void kill_enemy(Entity* entity) {
    entity->enemy->respawn_timer = get_random_between(5.0f, 15.0f);
    entity->enemy->is_dead = true;
    
    entity->is_visible = false;
}

void enemy_on_create(Entity* entity) {
    entity->enemy->respawn_timer = get_random_between(5.0f, 15.0f);
    entity->has_collider = true;
}

void enemy_on_destroy(Entity* entity) {

}

void enemy_on_update(Entity* entity) {
    if (entity->enemy->is_dead) {
        if ((entity->enemy->respawn_timer -= time.delta) <= 0.0f) {
            spawn_enemy(entity);
        }
    }
    else {
        entity->position    += entity->enemy->velocity * time.delta;
        entity->orientation -= 100.0f * time.delta;

        if ((entity->enemy->next_fire -= time.delta) <= 0.0f && !the_player->player->is_dead) {
            entity->enemy->next_fire = entity->enemy->fire_rate;

            float fire_angle = 0.0f;
            switch (entity->enemy->mode) {
                case Enemy_Mode::SMALL: {
                    fire_angle = get_angle(normalize(the_player->position - entity->position)) + get_random_between(-25.0f, 25.0f);
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

            Entity* laser = create_entity(Entity_Type::LASER);
            set_shooter(laser, entity);

            laser->position    = entity->position + (get_direction(fire_angle) * entity->sprite_size);
            laser->orientation = fire_angle;
        }
    }
}

void enemy_on_collision(Entity* us, Entity* them) {
    switch (them->type) {
        case Entity_Type::ASTEROID: {
            kill_enemy(us);
            destroy_entity(them);

            break;
        }
    }
}