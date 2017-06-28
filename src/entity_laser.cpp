struct Laser {
    int   shooter_id = -1;
    float lifetime   = 1.0f;
};

void set_shooter(Entity* laser, Entity* shooter) {
    laser->laser->shooter_id = shooter->id;
}

void laser_on_create(Entity* entity) {
    set_collider(entity, 0.1f);

    entity->sprite        = &laser_sprite;
    entity->sprite_size   = 0.75f;
    entity->sprite_offset = make_vector2(0.0f, -0.3f);

    if (get_random_chance(2)) {
        play_sound(&laser_01_sound);
    }
    else {
        play_sound(&laser_02_sound);
    }

    show_entity(entity);
}

void laser_on_destroy(Entity* entity) {

}

void laser_on_update(Entity* entity) {
    if ((entity->laser->lifetime -= time.delta) <= 0.0f) {
        destroy_entity(entity);
    }
    else {
        entity->position += get_direction(entity->orientation) * 15.0f * time.delta;
    }
}

void laser_on_collision(Entity* us, Entity* them) {
    Entity* shooter = find_entity(us->laser->shooter_id);
    switch (shooter->type) {
        case Entity_Type::PLAYER: {
            switch (them->type) {
                case Entity_Type::ASTEROID: {
                    add_score(shooter, them->asteroid->score);
                    destroy_entity(them);

                    destroy_entity(us);
                    break;
                }
                case Entity_Type::ENEMY: {
                    if (!them->enemy->is_dead) {
                        add_score(shooter, them->enemy->score);
                        kill_enemy(them);

                        destroy_entity(us);
                    }

                    break;
                }
            }

            break;
        }
        case Entity_Type::ENEMY: {
            switch (them->type) {
                case Entity_Type::ASTEROID: {
                    destroy_entity(them);
                    destroy_entity(us);

                    break;
                }
                case Entity_Type::PLAYER: {
                    if (!them->player->is_dead) {
                        kill_player(them);
                        destroy_entity(us);
                    }
                    
                    break;
                }
            }

            break;
        }
        default: {
            assert(false);
            break;
        }
    }
}