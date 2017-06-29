void set_asteroid_size(Asteroid* asteroid, Asteroid_Size size) {
    asteroid->size = size;

    switch (asteroid->size) {
        case Asteroid_Size::SMALL: {
            asteroid->entity->sprite      = &asteroid_small_sprite;
            asteroid->entity->sprite_size = 0.4f;

            asteroid->score          = 100;
            asteroid->velocity       = make_vector2(get_random_between(-3.0f, 3.0f), get_random_between(-3.0f, 3.0f));
            asteroid->rotation_speed = get_random_between(-300.0f, 300.0f);
                
            break;
        }
        case Asteroid_Size::MEDIUM: {
            asteroid->entity->sprite      = &asteroid_medium_sprite;
            asteroid->entity->sprite_size = 1.0f;

            asteroid->score          = 50;
            asteroid->velocity       = make_vector2(get_random_between(-1.5f, 1.5f), get_random_between(-1.5f, 1.5f));
            asteroid->rotation_speed = get_random_between(-150.0f, 150.0f);

            break;
        }
        case Asteroid_Size::LARGE: {
            asteroid->entity->sprite      = &asteroid_large_sprite;
            asteroid->entity->sprite_size = 2.0f;

            asteroid->score          = 20;
            asteroid->velocity       = make_vector2(get_random_between(-0.5f, 0.5f), get_random_between(-0.5f, 0.5f));
            asteroid->rotation_speed = get_random_between(-50.0f, 50.0f);

            break;
        }
        default: {
            assert(false);
            break;
        }
    }

    asteroid->entity->is_visible      = true;
    asteroid->entity->has_collider    = true;
    asteroid->entity->collider_radius = asteroid->entity->sprite_size / 2.0f;
}

void on_create(Asteroid* asteroid) {
    asteroid->entity->orientation = get_random_between(0.0f, 360.0f);
}

void on_destroy(Asteroid* asteroid) {
    switch (asteroid->size) {
        case Asteroid_Size::SMALL: {
            break;
        }
        case Asteroid_Size::MEDIUM: {
            for (int i = 0; i < 2; i++) {
                Asteroid* child_asteroid = create_entity(Entity_Type::ASTEROID)->asteroid;
                set_asteroid_size(child_asteroid, Asteroid_Size::SMALL);

                child_asteroid->entity->position = asteroid->entity->position;
            }

            break;
        }
        case Asteroid_Size::LARGE: {
            for (int i = 0; i < 2; i++) {
                Asteroid* child_asteroid = create_entity(Entity_Type::ASTEROID)->asteroid;
                set_asteroid_size(child_asteroid, Asteroid_Size::MEDIUM);

                child_asteroid->entity->position = asteroid->entity->position;
            }

            break;
        }
        default: {
            assert(false);
            break;
        }
    }

    play_sound(&kill_02_sound);
}

void on_update(Asteroid* asteroid) {
    asteroid->entity->position    += asteroid->velocity * time.delta;
    asteroid->entity->orientation += asteroid->rotation_speed * time.delta;
}

void on_collision(Asteroid* asteroid, Entity* them) {
    
}