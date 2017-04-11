const float PLAYER_SPEED    = 10.0f;
const float PLAYER_RADIUS   = 0.5f;

const float LASER_SPEED     = 15.0f;
const float LASER_RADIUS    = 0.1f;
const float LASER_LIFETIME  = 1.5f;

struct Player {
    Vector2 position;
    float   orientation;

    Vector2 velocity;
};

struct Laser {
    bool is_active;

    Vector2 position;
    float   orientation;
    float   timer;
};

enum Meteor_Size {
    MS_TINY,
    MS_SMALL,
    MS_MEDIUM,
    MS_BIG,
    MS_LARGE,
    MS_COUNT
};

const float MS_RADII[] = { 0.1f, 0.2f, 0.3f, 0.6f, 1.1f };

struct Meteor {
    bool is_active;

    Meteor_Size size;
    Sprite*     sprite;

    Vector2 position;
    float   orientation;

    Vector2 linear_velocity;
    float   angular_velocity;
};

struct Game {
    Sprite ship_sprite;
    Sprite laser_sprite;
    Sprite meteor_sprites_tiny[2];
    Sprite meteor_sprites_small[2];
    Sprite meteor_sprites_medium[2];
    Sprite meteor_sprites_big[4];
    Sprite meteor_sprites_large[4];

    Player player;

    Laser  lasers[32];
    Meteor meteors[32];

    float meteor_spawn_timer;
};

Game game;

void init_game() {
    game.ship_sprite                = load_sprite("data/sprites/player_ship.png");
    game.laser_sprite               = load_sprite("data/sprites/player_laser.png");
    game.meteor_sprites_tiny[0]     = load_sprite("data/sprites/meteor_tiny1.png");
    game.meteor_sprites_tiny[1]     = load_sprite("data/sprites/meteor_tiny2.png");
    game.meteor_sprites_small[0]    = load_sprite("data/sprites/meteor_small1.png");
    game.meteor_sprites_small[1]    = load_sprite("data/sprites/meteor_small2.png");
    game.meteor_sprites_medium[0]   = load_sprite("data/sprites/meteor_medium1.png");
    game.meteor_sprites_medium[1]   = load_sprite("data/sprites/meteor_medium2.png");
    game.meteor_sprites_big[0]      = load_sprite("data/sprites/meteor_big1.png");
    game.meteor_sprites_big[1]      = load_sprite("data/sprites/meteor_big2.png");
    game.meteor_sprites_big[2]      = load_sprite("data/sprites/meteor_big3.png");
    game.meteor_sprites_big[3]      = load_sprite("data/sprites/meteor_big4.png");
    game.meteor_sprites_large[0]    = load_sprite("data/sprites/meteor_large1.png");
    game.meteor_sprites_large[1]    = load_sprite("data/sprites/meteor_large2.png");
    game.meteor_sprites_large[2]    = load_sprite("data/sprites/meteor_large3.png");
    game.meteor_sprites_large[3]    = load_sprite("data/sprites/meteor_large4.png");
}

void start_game() {
    seed_random(0);

    game.player.position    = make_vector2();
    game.player.orientation = 0.0f;

    game.player.velocity = make_vector2();

    for (int i = 0; i < count_of(game.lasers); i++) {
        game.lasers[i].is_active = false;
    }

    for (int i = 0; i < count_of(game.meteors); i++) {
        game.meteors[i].is_active = false;
    }

    game.meteor_spawn_timer = 0.0f;
}

void stop_game() {

}

bool check_collision(Vector2 position_a, float radius_a, Vector2 position_b, float radius_b) {
    float distance_squared = square(position_a.x - position_b.x) + square(position_a.y - position_b.y);
    float radii_squared    = square(radius_a + radius_b);

    return distance_squared <= radii_squared;
}

void wrap_position(Vector2* position) {
    if (position->x < -HALF_WORLD_WIDTH) {
        position->x = HALF_WORLD_WIDTH;
    }

    if (position->x > HALF_WORLD_WIDTH) {
        position->x = -HALF_WORLD_WIDTH;
    }

    if (position->y < -HALF_WORLD_HEIGHT) {
        position->y = HALF_WORLD_HEIGHT;
    }

    if (position->y > HALF_WORLD_HEIGHT) {
        position->y = -HALF_WORLD_HEIGHT;
    }
}

void update_game() {
    if (input.escape.down) {
        switch_game_mode(GM_MENU);
    }

    if ((game.meteor_spawn_timer -= delta_time) <= 0.0f) {
        Meteor* meteor = NULL;
        for (int i = 0; i < count_of(game.meteors); i++) {
            if (game.meteors[i].is_active) continue;

            meteor = &game.meteors[i];
            break;
        }

        if (meteor) {
            meteor->is_active = true;

            meteor->size = (Meteor_Size) get_random_between(0, MS_COUNT - 1);
            switch (meteor->size) {
                case MS_TINY: {
                    int index = get_random_between(0, count_of(game.meteor_sprites_tiny) - 1);
                    meteor->sprite = &game.meteor_sprites_tiny[index];

                    break;
                }
                case MS_SMALL: {
                    int index = get_random_between(0, count_of(game.meteor_sprites_small) - 1);
                    meteor->sprite = &game.meteor_sprites_small[index];

                    break;
                }
                case MS_MEDIUM: {
                    int index = get_random_between(0, count_of(game.meteor_sprites_medium) - 1);
                    meteor->sprite = &game.meteor_sprites_medium[index];

                    break;
                }
                case MS_BIG: {
                    int index = get_random_between(0, count_of(game.meteor_sprites_big) - 1);
                    meteor->sprite = &game.meteor_sprites_big[index];

                    break;
                }
                case MS_LARGE: {
                    int index = get_random_between(0, count_of(game.meteor_sprites_large) - 1);
                    meteor->sprite = &game.meteor_sprites_large[index];

                    break;
                }
            }
            
            meteor->position.x = get_random_between(-HALF_WORLD_WIDTH, HALF_WORLD_WIDTH);
            meteor->position.y = get_random_between(-HALF_WORLD_HEIGHT, HALF_WORLD_HEIGHT);

            meteor->orientation = get_random_between(0.0f, 360.0f);
        }
        else {
            print("Failed to spawn meteor, no more meteor slots are available\n");
        }

        game.meteor_spawn_timer = 1.0f;
    }

    for (int i = 0; i < count_of(game.meteors); i++) {
        Meteor* meteor = &game.meteors[i];
        if (!meteor->is_active) continue;

        draw_sprite(meteor->sprite, meteor->position, 0.0f);
        draw_circle(meteor->position, MS_RADII[meteor->size], GREEN);
    }

    Vector2 acceleration = make_vector2();

    if (input.space.held) {
        acceleration = get_direction(game.player.orientation) * PLAYER_SPEED;
    }

    acceleration = acceleration - (game.player.velocity * 0.5f);

    game.player.velocity = game.player.velocity + (acceleration * delta_time);
    game.player.position = game.player.position + (game.player.velocity * delta_time);

    wrap_position(&game.player.position);

    float screen_x = (2.0f * input.mouse_x) / WINDOW_WIDTH - 1.0f;
    float screen_y = 1.0f - (2.0f * input.mouse_y) / WINDOW_HEIGHT;

    Vector2 world_position = make_inverse(world_projection) * make_vector2(screen_x, screen_y);
    Vector2 new_direction  = normalize(world_position - game.player.position);

    game.player.orientation = to_degrees(atan2f(new_direction.y, new_direction.x)) - 90.0f;

    for (int i = 0; i < count_of(game.meteors); i++) {
        Meteor* meteor = &game.meteors[i];
        if ()
    }

    if (check_collision(game.player.position, PLAYER_RADIUS, ))

    draw_sprite(&game.ship_sprite, game.player.position, game.player.orientation);
    draw_circle(game.player.position, PLAYER_RADIUS, GREEN);

    if (!input.space.held && input.left_mb.down) {
        Laser* laser = NULL;
        for (int i = 0; i < count_of(game.lasers); i++) {
            if (game.lasers[i].is_active) continue;

            laser = &game.lasers[i];
            break;
        }

        if (laser) {
            laser->is_active = true;

            laser->position    = game.player.position + (get_direction(game.player.orientation) * 0.75f);
            laser->orientation = game.player.orientation;
            laser->timer       = LASER_LIFETIME;
        }
        else {
            print("Failed to spawn laser, no more laser slots are available\n");
        }
    }

    for (int i = 0; i < count_of(game.lasers); i++) {
        Laser* laser = &game.lasers[i];
        if (!laser->is_active) continue;

        if ((laser->timer -= delta_time) <= 0.0f) {
            laser->is_active = false;
            continue;
        }

        laser->position = laser->position + (get_direction(laser->orientation) * LASER_SPEED * delta_time);
        wrap_position(&laser->position);

        if (check_collision(game.player.position, PLAYER_RADIUS, laser->position, LASER_RADIUS)) {
            laser->is_active = false;
            continue;
        }

        draw_sprite(&game.laser_sprite, laser->position - get_direction(laser->orientation) * 0.2f, laser->orientation);
        draw_circle(laser->position, LASER_RADIUS, GREEN);
    }
}