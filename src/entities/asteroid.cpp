#if AS_HEADER

struct Asteroid {
    Entity* entity = null;

    Asteroid_Size size;
    u32 score = 0;

    Vector2 velocity;
    f32 rotation_speed = 0.0f;
};

void on_create(Asteroid* asteroid);
void on_destroy(Asteroid* asteroid);
void on_update(Asteroid* asteroid);
void on_collision(Asteroid* asteroid, Entity* them);

#else

void set_asteroid_size(Asteroid* asteroid, Asteroid_Size size) {
    asteroid->size = size;

    switch (asteroid->size) {
        case ASTEROID_SIZE_SMALL: {
            set_sprite(asteroid->entity, get_asteroid_sprite(ASTEROID_SIZE_SMALL), 0.4f);
            set_collider(asteroid->entity, 0.2f);

            asteroid->score = 100;

            asteroid->velocity       = make_vector2(get_random_between(-3.0f, 3.0f), get_random_between(-3.0f, 3.0f));
            asteroid->rotation_speed = get_random_between(-300.0f, 300.0f);
                
            break;
        }
        case ASTEROID_SIZE_MEDIUM: {
            set_sprite(asteroid->entity, get_asteroid_sprite(ASTEROID_SIZE_MEDIUM), 1.0f);
            set_collider(asteroid->entity, 0.5f);
            
            asteroid->score = 50;

            asteroid->velocity       = make_vector2(get_random_between(-1.5f, 1.5f), get_random_between(-1.5f, 1.5f));
            asteroid->rotation_speed = get_random_between(-150.0f, 150.0f);

            break;
        }
        case ASTEROID_SIZE_LARGE: {
            set_sprite(asteroid->entity, get_asteroid_sprite(ASTEROID_SIZE_LARGE), 2.0f);
            set_collider(asteroid->entity, 1.0f);
            
            asteroid->score = 20;

            asteroid->velocity       = make_vector2(get_random_between(-0.5f, 0.5f), get_random_between(-0.5f, 0.5f));
            asteroid->rotation_speed = get_random_between(-50.0f, 50.0f);

            break;
        }
        invalid_default_case();
    }
}

void spawn_children(Asteroid* asteroid) {
    for (u32 i = 0; i < 2; i++) {
        switch (asteroid->size) {
            case ASTEROID_SIZE_SMALL: {
                break;
            }
            case ASTEROID_SIZE_MEDIUM: {
                Asteroid* child_asteroid = create_entity(ENTITY_TYPE_ASTEROID)->asteroid;

                child_asteroid->entity->position = asteroid->entity->position;
                set_asteroid_size(child_asteroid, ASTEROID_SIZE_SMALL);

                break;
            }
            case ASTEROID_SIZE_LARGE: {
                Asteroid* child_asteroid = create_entity(ENTITY_TYPE_ASTEROID)->asteroid;

                child_asteroid->entity->position = asteroid->entity->position;
                set_asteroid_size(child_asteroid, ASTEROID_SIZE_MEDIUM);

                break;
            }
            invalid_default_case();
        }
    }

    play_sound(get_kill_sound());
}

void on_create(Asteroid* asteroid) {
    asteroid->entity->orientation = get_random_between(-360.0f, 360.0f);
}

void on_destroy(Asteroid* asteroid) {
    
}

void on_update(Asteroid* asteroid) {
    asteroid->entity->position    += asteroid->velocity * timers.delta;
    asteroid->entity->orientation += asteroid->rotation_speed * timers.delta;
}

void on_collision(Asteroid* asteroid, Entity* them) {
    
}

#endif