const bool DRAW_COLLIDERS           = false;
const float NEW_LEVEL_WAIT_TIME     = 2.5f;

const float PLAYER_MOVE_SPEED       = 10.0f;
const float PLAYER_TURN_SPEED       = 15.0f;
const float PLAYER_RADIUS           = 0.5f;

const float LASER_SPEED             = 15.0f;
const float LASER_RADIUS            = 0.1f;
const float LASER_LIFETIME          = 0.75f;

const int METEOR_SCORE_SMALL        = 75;
const int METEOR_SCORE_MEDIUM       = 50;
const int METEOR_SCORE_LARGE        = 25;

const float METEOR_RADIUS_SMALL     = 0.2f;
const float METEOR_RADIUS_MEDIUM    = 0.5f;
const float METEOR_RADIUS_LARGE     = 1.0f;

const float METEOR_SPEED_SMALL      = 5.0f;
const float METEOR_SPEED_MEDIUM     = 3.0f;
const float METEOR_SPEED_LARGE      = 1.0f;

const float PARTICLE_FPS            = 1.0f / 30.0f;

struct Player {
    Vector2 position;
    float   orientation;

    Vector2 velocity;
    int score;
};

struct Laser {
    bool is_active;

    Vector2 position;
    float   orientation;
    float   timer;
};

enum Meteor_Size {
    MS_SMALL,
    MS_MEDIUM,
    MS_LARGE,
    MS_COUNT
};

struct Meteor {
    bool is_active;
    bool is_destroyed;

    Meteor_Size size;
    
    Sprite* sprite;
    float   radius;
    int     score;

    Vector2 position;
    float   orientation;

    Vector2 velocity;
};

struct Particle {
    bool is_active;

    int   sprite_index;
    float sprite_timer;

    Vector2 position;
    float   orientation;
};

struct Particle_Emitter {
    bool is_active;
    bool is_emitting;
    
    Vector2 position;
    float   radius;
    float   duration;
    float   rate;
    float   timer;

    Particle particles[32];
};

struct Asteroids {
    Sprite ship_sprite;
    Sprite laser_sprite;
    Sprite thrust_sprite;

    Sprite meteor_sprites_small[2];
    Sprite meteor_sprites_medium[4];
    Sprite meteor_sprites_large[4];

    Sprite particle_sprites[9];

    Player              player;
    Laser               lasers[32];
    Meteor              meteors[32];
    Particle_Emitter    particle_emitters[32];

    bool  is_end_of_level;
    float new_level_timer;
};

Asteroids asteroids;

void init_asteroids() {
    asteroids.ship_sprite                = load_sprite("data/sprites/player_ship.png");
    asteroids.laser_sprite               = load_sprite("data/sprites/player_laser.png");
    asteroids.thrust_sprite              = load_sprite("data/sprites/player_thrust.png");

    asteroids.meteor_sprites_small[0]    = load_sprite("data/sprites/meteor_small_01.png");
    asteroids.meteor_sprites_small[1]    = load_sprite("data/sprites/meteor_small_02.png");

    asteroids.meteor_sprites_medium[0]   = load_sprite("data/sprites/meteor_medium_01.png");
    asteroids.meteor_sprites_medium[1]   = load_sprite("data/sprites/meteor_medium_02.png");
    asteroids.meteor_sprites_medium[2]   = load_sprite("data/sprites/meteor_medium_03.png");
    asteroids.meteor_sprites_medium[3]   = load_sprite("data/sprites/meteor_medium_04.png");

    asteroids.meteor_sprites_large[0]    = load_sprite("data/sprites/meteor_large_01.png");
    asteroids.meteor_sprites_large[1]    = load_sprite("data/sprites/meteor_large_02.png");
    asteroids.meteor_sprites_large[2]    = load_sprite("data/sprites/meteor_large_03.png");
    asteroids.meteor_sprites_large[3]    = load_sprite("data/sprites/meteor_large_04.png");

    asteroids.particle_sprites[0]        = load_sprite("data/sprites/particle_01.png");
    asteroids.particle_sprites[1]        = load_sprite("data/sprites/particle_02.png");
    asteroids.particle_sprites[2]        = load_sprite("data/sprites/particle_03.png");
    asteroids.particle_sprites[3]        = load_sprite("data/sprites/particle_04.png");
    asteroids.particle_sprites[4]        = load_sprite("data/sprites/particle_05.png");
    asteroids.particle_sprites[5]        = load_sprite("data/sprites/particle_06.png");
    asteroids.particle_sprites[6]        = load_sprite("data/sprites/particle_07.png");
    asteroids.particle_sprites[7]        = load_sprite("data/sprites/particle_08.png");
    asteroids.particle_sprites[8]        = load_sprite("data/sprites/particle_09.png");
}

void spawn_meteor(Meteor_Size size, Vector2 position) {
    Meteor* meteor = NULL;
    for (int i = 0; i < count_of(asteroids.meteors); i++) {
        if (asteroids.meteors[i].is_active) continue;

        meteor = &asteroids.meteors[i];
        break;
    }

    if (!meteor) {
        print("Failed to spawn meteor, no more meteor slots are available\n");
        return;
    }

    meteor->is_active    = true;
    meteor->is_destroyed = false;

    meteor->size = size;
    
    meteor->position = position;
    meteor->orientation = get_random_between(0.0f, 360.0f);

    Vector2 direction = make_vector2(get_random_between(-1.0f, 1.0f), get_random_between(-1.0f, 1.0f));
    direction = normalize(direction);

    switch (meteor->size) {
        case MS_SMALL: {
            int index = get_random_between(0, count_of(asteroids.meteor_sprites_small) - 1);
            meteor->sprite = &asteroids.meteor_sprites_small[index];

            meteor->radius = METEOR_RADIUS_SMALL;
            meteor->score  = METEOR_SCORE_SMALL;

            meteor->velocity = direction * METEOR_SPEED_SMALL;

            break;
        }
        case MS_MEDIUM: {
            int index = get_random_between(0, count_of(asteroids.meteor_sprites_medium) - 1);
            meteor->sprite = &asteroids.meteor_sprites_medium[index];

            meteor->radius = METEOR_RADIUS_MEDIUM;
            meteor->score  = METEOR_SCORE_MEDIUM;

            meteor->velocity = direction * METEOR_SPEED_MEDIUM;

            break;
        }
        case MS_LARGE: {
            int index = get_random_between(0, count_of(asteroids.meteor_sprites_large) - 1);
            meteor->sprite = &asteroids.meteor_sprites_large[index];

            meteor->radius = METEOR_RADIUS_LARGE;
            meteor->score  = METEOR_SCORE_LARGE;

            meteor->velocity = direction * METEOR_SPEED_LARGE;

            break;
        }
    }
}

void begin_level(int meteors) {
    for (int i = 0; i < meteors; i++) {
        Vector2 position = make_vector2();

        if (get_random_between(0, 1)) {
            position.x = get_random_between(HALF_VIEWPORT_WIDTH - QUARTER_VIEWPORT_WIDTH, HALF_VIEWPORT_WIDTH);
        }
        else {
            position.x = get_random_between(-HALF_VIEWPORT_WIDTH, -HALF_VIEWPORT_WIDTH + QUARTER_VIEWPORT_WIDTH);
        }

        if (get_random_between(0, 1)) {
            position.y = get_random_between(HALF_VIEWPORT_HEIGHT - QUARTER_VIEWPORT_HEIGHT, HALF_VIEWPORT_HEIGHT);
        }
        else {
            position.y = get_random_between(-HALF_VIEWPORT_HEIGHT, -HALF_VIEWPORT_HEIGHT + QUARTER_VIEWPORT_HEIGHT);
        }

        spawn_meteor(MS_LARGE, position);
    }

    asteroids.is_end_of_level = false;
}

void start_asteroids() {
    asteroids.player.position    = make_vector2();
    asteroids.player.orientation = 0.0f;

    asteroids.player.velocity = make_vector2();
    asteroids.player.score    = 0;

    for (int i = 0; i < count_of(asteroids.lasers); i++) {
        asteroids.lasers[i].is_active = false;
    }

    for (int i = 0; i < count_of(asteroids.meteors); i++) {
        asteroids.meteors[i].is_active    = false;
        asteroids.meteors[i].is_destroyed = false;
    }

    for (int i = 0; i < count_of(asteroids.particle_emitters); i++) {
        asteroids.particle_emitters[i].is_active = false;
    }

    begin_level(5);
}

bool check_collision(Vector2 position_a, float radius_a, Vector2 position_b, float radius_b) {
    float distance_squared = square(position_a.x - position_b.x) + square(position_a.y - position_b.y);
    float radii_squared    = square(radius_a + radius_b);

    return distance_squared <= radii_squared;
}

void wrap_position(Vector2* position) {
    if (position->x < -HALF_VIEWPORT_WIDTH) {
        position->x = HALF_VIEWPORT_WIDTH;
    }

    if (position->x > HALF_VIEWPORT_WIDTH) {
        position->x = -HALF_VIEWPORT_WIDTH;
    }

    if (position->y < -HALF_VIEWPORT_HEIGHT) {
        position->y = HALF_VIEWPORT_HEIGHT;
    }

    if (position->y > HALF_VIEWPORT_HEIGHT) {
        position->y = -HALF_VIEWPORT_HEIGHT;
    }
}

void emit_particles(Vector2 position, float radius, float duration, float rate) {
    Particle_Emitter* emitter = NULL;
    for (int i = 0; i < count_of(asteroids.particle_emitters); i++) {
        if (asteroids.particle_emitters[i].is_active) continue;
        
        emitter = &asteroids.particle_emitters[i];
        break;
    }

    if (emitter) {
        emitter->is_active = true;
        emitter->is_emitting = true;

        emitter->position = position;
        emitter->radius   = radius;
        emitter->duration = duration;
        emitter->rate     = rate;
        emitter->timer    = 0.0f;

        for (int i = 0; i < count_of(emitter->particles); i++) {
            emitter->particles[i].is_active = false;
        }
    }
    else {
        print("Failed to emit particles, no more particle emitters are available\n");
    }
}

void update_asteroids() {
    if (input.escape.down) {
        switch_game_mode(GM_MENU);
    }

    Vector2 acceleration = make_vector2();

    if (input.space.held) {
        acceleration = get_direction(asteroids.player.orientation) * PLAYER_MOVE_SPEED;
    }

    acceleration = acceleration - (asteroids.player.velocity * 0.5f);

    asteroids.player.velocity = asteroids.player.velocity + (acceleration * delta_time);
    asteroids.player.position = asteroids.player.position + (asteroids.player.velocity * delta_time);

    wrap_position(&asteroids.player.position);

    float screen_x = (2.0f * input.mouse_x) / SCREEN_WIDTH - 1.0f;
    float screen_y = 1.0f - (2.0f * input.mouse_y) / SCREEN_HEIGHT;

    Vector2 mouse_world_position = make_inverse(world_projection) * make_vector2(screen_x, screen_y);
    
    Vector2 current_direction = get_direction(asteroids.player.orientation);
    Vector2 desired_direction = normalize(mouse_world_position - asteroids.player.position);

    Vector2 new_direction = lerp(current_direction, PLAYER_TURN_SPEED * delta_time, desired_direction);
    asteroids.player.orientation = to_degrees(atan2f(new_direction.y, new_direction.x)) - 90.0f;

    draw_sprite(&asteroids.ship_sprite, asteroids.player.position, asteroids.player.orientation);

    if (input.space.held) {
        Matrix4 player_transform = make_transform(asteroids.player.position, asteroids.player.orientation);

        Matrix4 left_transform  = make_transform(make_vector2(-0.25f, -0.4f), 180.0f);
        Matrix4 right_transform = make_transform(make_vector2( 0.25f, -0.4f), 180.0f);

        draw_sprite(&asteroids.thrust_sprite, player_transform * left_transform);
        draw_sprite(&asteroids.thrust_sprite, player_transform * right_transform);
    }

    if (DRAW_COLLIDERS) {
        draw_circle(asteroids.player.position, PLAYER_RADIUS, GREEN);
    }

    if (!input.space.held && input.left_mb.down) {
        Laser* laser = NULL;
        for (int i = 0; i < count_of(asteroids.lasers); i++) {
            if (asteroids.lasers[i].is_active) continue;

            laser = &asteroids.lasers[i];
            break;
        }

        if (laser) {
            laser->is_active = true;

            laser->position    = asteroids.player.position + (get_direction(asteroids.player.orientation) * 0.75f);
            laser->orientation = asteroids.player.orientation;
            laser->timer       = LASER_LIFETIME;
        }
        else {
            print("Failed to spawn laser, no more laser slots are available\n");
        }
    }

    for (int i = 0; i < count_of(asteroids.lasers); i++) {
        Laser* laser = &asteroids.lasers[i];
        if (!laser->is_active) continue;

        if ((laser->timer -= delta_time) <= 0.0f) {
            laser->is_active = false;
            continue;
        }

        laser->position = laser->position + (get_direction(laser->orientation) * LASER_SPEED * delta_time);
        wrap_position(&laser->position);

        draw_sprite(&asteroids.laser_sprite, laser->position - get_direction(laser->orientation) * 0.2f, laser->orientation);
        
        if (DRAW_COLLIDERS) {
            draw_circle(laser->position, LASER_RADIUS, GREEN);
        }
    }
    
    for (int i = 0; i < count_of(asteroids.meteors); i++) {
        Meteor* meteor = &asteroids.meteors[i];
        
        if (!meteor->is_active)    continue;
        if (!meteor->is_destroyed) continue;

        asteroids.player.score += meteor->score;
        
        if (meteor->size > MS_SMALL) {
            int children = get_random_between(1, 3);
            for (int i = 0; i < children; i++) {
                spawn_meteor((Meteor_Size) (meteor->size - 1), meteor->position);
            }
        }

        meteor->is_active = false;
    }

    for (int i = 0; i < count_of(asteroids.meteors); i++) {
        Meteor* meteor = &asteroids.meteors[i];
        if (!meteor->is_active) continue;

        meteor->position = meteor->position + meteor->velocity * delta_time;
        wrap_position(&meteor->position);

        if (check_collision(asteroids.player.position, PLAYER_RADIUS, meteor->position, meteor->radius)) {
            // switch_game_mode(GM_MENU);
        }

        for (int i = 0; i < count_of(asteroids.lasers); i++) {
            Laser* laser = &asteroids.lasers[i];
            if (!laser->is_active) continue;

            if (check_collision(meteor->position, meteor->radius, laser->position, LASER_RADIUS)) {
                meteor->is_destroyed = true;
                laser->is_active     = false;

                Vector2 position = meteor->position + (normalize(laser->position - meteor->position) * meteor->radius);
                emit_particles(position, 0.25f, 0.25f, 5.0f);

                break;
            }
        }

        draw_sprite(meteor->sprite, meteor->position, 0.0f);

        if (DRAW_COLLIDERS) {
            draw_circle(meteor->position, meteor->radius, GREEN);
        }
    }

    bool has_meteors = false;

    for (int i = 0; i < count_of(asteroids.meteors); i++) {
        Meteor* meteor = &asteroids.meteors[i];
        if (!meteor->is_active) continue;

        has_meteors = true;
    }

    if (!has_meteors) {
        if (!asteroids.is_end_of_level) {
            asteroids.is_end_of_level = true;
            asteroids.new_level_timer = NEW_LEVEL_WAIT_TIME;
        }

        if ((asteroids.new_level_timer -= delta_time) <= 0.0f) {
            begin_level(5);
        }
    }

    for (int i = 0; i < count_of(asteroids.particle_emitters); i++) {
        Particle_Emitter* emitter = &asteroids.particle_emitters[i];
        if (!emitter->is_active) continue;

        if (emitter->is_emitting) {
            if ((emitter->duration -= delta_time) <= 0.0f) {
                emitter->is_emitting = false;
            }
            else {
                if ((emitter->timer -= delta_time) <= 0.0f) {
                    Particle* particle = NULL;
                    for (int i = 0; i < count_of(emitter->particles); i++) {
                        if (emitter->particles[i].is_active) continue;

                        particle = &emitter->particles[i];
                        break;
                    }

                    if (particle) {
                        particle->is_active = true;
                        
                        particle->sprite_index = get_random_between((int) count_of(asteroids.particle_sprites) - 3, (int) count_of(asteroids.particle_sprites) - 1);
                        particle->sprite_timer = PARTICLE_FPS;

                        float offset_x = get_random_between(-emitter->radius, emitter->radius);
                        float offset_y = get_random_between(-emitter->radius, emitter->radius);

                        particle->position    = emitter->position + make_vector2(offset_x, offset_y);
                        particle->orientation = get_random_between(0.0f, 360.0f);
                    }
                    else {
                        print("Failed to spawn particle, no more particle slots are available\n");
                    }

                    emitter->timer = 1.0f / emitter->rate;
                }
            }
        }
        
        bool has_active_particles = false;
        for (int i = 0; i < count_of(emitter->particles); i++) {
            Particle* particle = &emitter->particles[i];
            if (!particle->is_active) continue;

            if ((particle->sprite_timer -= delta_time) <= 0.0f) {
                if (particle->sprite_index == 0) {
                    particle->is_active = false;
                    continue;
                }

                particle->sprite_timer = PARTICLE_FPS;
                particle->sprite_index--;
            }

            draw_sprite(&asteroids.particle_sprites[particle->sprite_index], particle->position, particle->orientation);
            has_active_particles = true;
        }

        if (!emitter->is_emitting && !has_active_particles) {
            emitter->is_active = false;
        }
    }

    draw_text(format_string("Score: %i", asteroids.player.score), make_vector2(16.0f, 16.0f));
}