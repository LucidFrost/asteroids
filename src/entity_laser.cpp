void on_create(Laser* laser) {
    laser->entity->has_collider = true;
    laser->entity->collider_radius = 0.1f;

    laser->entity->sprite        = &laser_sprite;
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
    if ((laser->lifetime -= time.delta) <= 0.0f) {
        destroy_entity(laser->entity);
    }
    else {
        laser->entity->position += get_direction(laser->entity->orientation) * 15.0f * time.delta;
    }
}

void on_collision(Laser* laser, Entity* them) {
    switch (laser->shooter->type) {
        case Entity_Type::PLAYER: {
            switch (them->type) {
                case Entity_Type::ASTEROID: {
                    add_score(laser->shooter->player, them->asteroid->score);
                    destroy_entity(them);

                    destroy_entity(laser->entity);
                    break;
                }
                case Entity_Type::ENEMY: {
                    if (!them->enemy->is_dead) {
                        add_score(laser->shooter->player, them->enemy->score);
                        kill_enemy(them->enemy);

                        destroy_entity(laser->entity);
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
                    destroy_entity(laser->entity);

                    break;
                }
                case Entity_Type::PLAYER: {
                    if (!them->player->is_dead) {
                        kill_player(them->player);
                        destroy_entity(laser->entity);
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