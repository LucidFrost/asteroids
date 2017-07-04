#if AS_HEADER
    struct Laser {
        Entity* entity = null;
        u32 shooter_id = 0;

        f32 lifetime = 1.0f;
    };

    void on_create(Laser* laser);
    void on_destroy(Laser* laser);
    void on_update(Laser* laser);
    void on_collision(Laser* laser, Entity* them);
#else
    void on_create(Laser* laser) {
        laser->entity->has_collider = true;
        laser->entity->collider_radius = 0.1f;

        laser->entity->sprite_size   = 0.75f;
        laser->entity->sprite_offset = make_vector2(0.0f, -0.3f);
        laser->entity->is_visible    = true;

        if (get_random_chance(2)) {
            play_sound(&laser_01_sound);
        }
        else {
            play_sound(&laser_02_sound);
        }
    }

    void on_destroy(Laser* laser) {

    }

    void on_update(Laser* laser) {
        if ((laser->lifetime -= timers.delta) <= 0.0f) {
            destroy_entity(laser->entity);
        }
        else {
            laser->entity->position += get_direction(laser->entity->orientation) * 15.0f * timers.delta;
        }
    }

    void on_collision(Laser* laser, Entity* them) {
        Entity* shooter = find_entity(laser->shooter_id);

        switch (them->type) {
            case ENTITY_TYPE_ASTEROID: {
                if (shooter) {
                    switch (shooter->type) {
                        case ENTITY_TYPE_PLAYER: {
                            add_score(them->asteroid->score);
                            break;
                        }
                        case ENTITY_TYPE_ENEMY: {
                            break;
                        }
                        invalid_default_case();
                    }
                }

                spawn_children(them->asteroid);
                destroy_entity(them);

                destroy_entity(laser->entity);
                break;
            }
            case ENTITY_TYPE_PLAYER: {
                if (shooter && shooter->type == ENTITY_TYPE_ENEMY) {
                    kill_player();
                    destroy_entity(laser->entity);
                }

                break;
            }
            case ENTITY_TYPE_ENEMY: {
                if (shooter && shooter->type == ENTITY_TYPE_PLAYER) {
                    add_score(them->enemy->score);
                    kill_enemy();

                    destroy_entity(laser->entity);
                }

                break;
            }
        }
    }
#endif