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

void init_laser(Laser* laser, Laser_Color color, Entity* shooter, f32 angle) {
    set_sprite(laser->entity, get_laser_sprite(color), 0.75f, 0, make_vector2(0.0f, -0.3f));
    laser->shooter_id = shooter->id;

    laser->entity->position    = shooter->position + (get_direction(angle) * 0.75f);
    laser->entity->orientation = angle;
}

void on_create(Laser* laser) {
    set_collider(laser->entity, 0.1f);
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
            if (shooter && shooter->type == ENTITY_TYPE_PLAYER) {
                add_score(them->asteroid->score);
            }

            spawn_children(them->asteroid);

            destroy_entity(them);
            destroy_entity(laser->entity);

            break;
        }
        case ENTITY_TYPE_PLAYER: {
            if (shooter && shooter->type == ENTITY_TYPE_ENEMY) {
                destroy_entity(laser->entity);
                damage_player(shooter->player);
            }

            break;
        }
        case ENTITY_TYPE_ENEMY: {
            if (shooter && shooter->type == ENTITY_TYPE_PLAYER) {
                add_score(them->enemy->score);

                destroy_entity(laser->entity);
                kill_enemy();
            }

            break;
        }
    }
}

#endif