enum struct Asteroid_Size {
    NONE,
    SMALL,
    MEDIUM,
    LARGE
};

struct Asteroid {
    Asteroid_Size size = Asteroid_Size::NONE;
    int score = 0;

    Vector2 velocity;
};

void set_asteroid_size(Entity* entity, Asteroid_Size size) {
    entity->asteroid->size = size;

    switch (entity->asteroid->size) {
        case Asteroid_Size::SMALL: {
            entity->collider_radius = 0.2f;
            
            entity->sprite      = &asteroid_small_sprite;
            entity->sprite_size = 0.4f;

            entity->asteroid->score = 100;

            entity->asteroid->velocity = make_vector2(
                get_random_between(-3.0f, 3.0f), 
                get_random_between(-3.0f, 3.0f));
                
            break;
        }
        case Asteroid_Size::MEDIUM: {
            entity->collider_radius = 0.5f;

            entity->sprite      = &asteroid_medium_sprite;
            entity->sprite_size = 1.0f;

            entity->asteroid->score = 50;

            entity->asteroid->velocity = make_vector2(
                get_random_between(-1.5f, 1.5f), 
                get_random_between(-1.5f, 1.5f));

            break;
        }
        case Asteroid_Size::LARGE: {
            entity->collider_radius = 1.0f;

            entity->sprite      = &asteroid_large_sprite;
            entity->sprite_size = 2.0f;
            
            entity->asteroid->score = 20;

            entity->asteroid->velocity = make_vector2(
                get_random_between(-0.5f, 0.5f), 
                get_random_between(-0.5f, 0.5f));

            break;
        }
        default: {
            assert(false);
            break;
        }
    }
}

void asteroid_on_create(Entity* entity) {
    entity->orientation  = get_random_between(0.0f, 360.0f);
    entity->has_collider = true;
}

void asteroid_on_destroy(Entity* entity) {
    switch (entity->asteroid->size) {
        case Asteroid_Size::SMALL: {
            break;
        }
        case Asteroid_Size::MEDIUM: {
            for (int i = 0; i < 2; i++) {
                Entity* asteroid = create_entity(Entity_Type::ASTEROID);
                set_asteroid_size(asteroid, Asteroid_Size::SMALL);

                asteroid->position = entity->position;
            }

            break;
        }
        case Asteroid_Size::LARGE: {
            for (int i = 0; i < 2; i++) {
                Entity* asteroid = create_entity(Entity_Type::ASTEROID);
                set_asteroid_size(asteroid, Asteroid_Size::MEDIUM);

                asteroid->position = entity->position;
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

void asteroid_on_update(Entity* entity) {
    entity->position += entity->asteroid->velocity * time.delta;
}

void asteroid_on_collision(Entity* us, Entity* them) {
    
}