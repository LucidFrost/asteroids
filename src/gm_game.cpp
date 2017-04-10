const float PLAYER_SPEED    = 10.0f;
const float PLAYER_TURN     = 350.0f;
const float PLAYER_RADIUS   = 0.5f;

const float LASER_SPEED     = 15.0f;
const float LASER_LIFETIME  = 1.5f;
const float LASER_RADIUS    = 0.1f;

struct Player {
    Vector2 position;
    Vector2 velocity;
    float   orientation;
};

struct Laser {
    bool is_active;

    Vector2 position;
    float   orientation;
    float   timer;
};

struct Meteor {
    bool is_active;

    Vector2 position;
    Vector2 velocity;
    float   orientation;
};

struct Game {
    Sprite ship_sprite;
    Sprite laser_sprite;

    Player player;
    Laser  lasers[32];
    Meteor meteors[32];
};

Game game;

bool check_collision(Vector2 position_a, float radius_a, Vector2 position_b, float radius_b) {
    float distance_squared = square(position_a.x - position_b.x) + square(position_a.y - position_b.y);
    float radii_squared    = square(radius_a + radius_b);

    return distance_squared <= radii_squared;
}

void wrap_position(Vector2* position) {
    if (position->x < -WORLD_HALF_WIDTH) {
        position->x = WORLD_HALF_WIDTH;
    }

    if (position->x > WORLD_HALF_WIDTH) {
        position->x = -WORLD_HALF_WIDTH;
    }

    if (position->y < -WORLD_HALF_HEIGHT) {
        position->y = WORLD_HALF_HEIGHT;
    }

    if (position->y > WORLD_HALF_HEIGHT) {
        position->y = -WORLD_HALF_HEIGHT;
    }
}

void init_game() {
    game.ship_sprite  = load_sprite("data/player_ship.png");  // @leak
    game.laser_sprite = load_sprite("data/player_laser.png"); // @leak

    game.player.position    = make_vector2();
    game.player.velocity    = make_vector2();
    game.player.orientation = 0.0f;

    for (int i = 0; i < count_of(game.lasers); i++) {
        game.lasers[i].is_active = false;
    }

    for (int i = 0; i < count_of(game.meteors); i++) {
        game.meteors[i].is_active = false;
    }
}

void update_game() {
    if (input.escape.down) {
        switch_game_mode(GM_MENU);
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

    draw_sprite(&game.ship_sprite, game.player.position, game.player.orientation);
    draw_circle(game.player.position, PLAYER_RADIUS, GREEN);

    if (!input.space.held && input.left_mb.down) {
        Laser* laser = NULL;
        for (int i = 0; i < count_of(game.lasers); i++) {
            if (!game.lasers[i].is_active) {
                laser = &game.lasers[i];
                break;
            }
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
        if (laser->is_active) {
            if ((laser->timer -= delta_time) <= 0.0f) {
                laser->is_active = false;
            }
            else {
                laser->position = laser->position + (get_direction(laser->orientation) * LASER_SPEED * delta_time);
                wrap_position(&laser->position);

                if (check_collision(game.player.position, PLAYER_RADIUS, laser->position, LASER_RADIUS)) {
                    laser->is_active = false;
                }
                else {
                    draw_sprite(&game.laser_sprite, laser->position - get_direction(laser->orientation) * 0.2f, laser->orientation);
                    draw_circle(laser->position, LASER_RADIUS, GREEN);
                }
            }
        }
    }
}