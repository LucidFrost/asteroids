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
    float   rotation_speed = 0.0f;
};

void set_asteroid_size(Entity* entity, Asteroid_Size size) {
    entity->asteroid->size = size;

    switch (entity->asteroid->size) {
        case Asteroid_Size::SMALL: {
            entity->sprite             = &asteroid_small_sprite;
            entity->sprite_size        = 0.4f;
            entity->asteroid->score    = 100;
            entity->asteroid->velocity = make_vector2(get_random_between(-3.0f, 3.0f), get_random_between(-3.0f, 3.0f));
            entity->asteroid->rotation_speed = get_random_between(-300.0f, 300.0f);
                
            break;
        }
        case Asteroid_Size::MEDIUM: {
            entity->sprite             = &asteroid_medium_sprite;
            entity->sprite_size        = 1.0f;
            entity->asteroid->score    = 50;
            entity->asteroid->velocity = make_vector2(get_random_between(-1.5f, 1.5f), get_random_between(-1.5f, 1.5f));
            entity->asteroid->rotation_speed = get_random_between(-150.0f, 150.0f);

            break;
        }
        case Asteroid_Size::LARGE: {
            entity->sprite             = &asteroid_large_sprite;
            entity->sprite_size        = 2.0f;
            entity->asteroid->score    = 20;
            entity->asteroid->velocity = make_vector2(get_random_between(-0.5f, 0.5f), get_random_between(-0.5f, 0.5f));
            entity->asteroid->rotation_speed = get_random_between(-50.0f, 50.0f);

            break;
        }
        default: {
            assert(false);
            break;
        }
    }

    set_collider(entity, entity->sprite_size / 2.0f);
    show_entity(entity);
}

void asteroid_on_create(Entity* entity) {
    entity->orientation  = get_random_between(0.0f, 360.0f);
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
    entity->orientation += entity->asteroid->rotation_speed * time.delta;
}

void asteroid_on_collision(Entity* us, Entity* them) {
    
}