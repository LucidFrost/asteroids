#if AS_HEADER

struct Player {
    Entity* entity = null;

    Entity* left_thrust  = null;
    Entity* right_thrust = null;

    Vector2 velocity;
    Vector2 desired_direction;

    i32 last_mouse_x = 0;
    i32 last_mouse_y = 0;

    Ship_Type  ship_type;
    Ship_Color ship_color;

    Damage_Type damage_type;
    Entity* damage  = null;
    bool is_damaged = false;

    f32 invincibility_timer = 0.0f;
    bool is_invincible = false;
};

void on_create(Player* player);
void on_destroy(Player* player);
void on_update(Player* player);
void on_collision(Player* player, Entity* them);

void add_score(u32 score);
void damage_player(Player* player);

#else

void kill_player();

void init_player(Player* player, Ship_Type ship_type, Ship_Color ship_color) {
    set_sprite(player->entity, get_ship_sprite(ship_type, ship_color));

    player->ship_type  = ship_type;
    player->ship_color = ship_color;
}

void damage_player(Player* player) {
    if (player->is_invincible) return;

    if (player->is_damaged) {
        switch (player->damage_type) {
            case DAMAGE_TYPE_SMALL: {
                player->damage_type = DAMAGE_TYPE_MEDIUM;
                break;
            }
            case DAMAGE_TYPE_MEDIUM: {
                player->damage_type = DAMAGE_TYPE_LARGE;
                break;
            }
            case DAMAGE_TYPE_LARGE: {
                kill_player();
                break;
            }
            invalid_default_case();
        }
    }
    else {
        player->damage_type = DAMAGE_TYPE_SMALL;
        player->is_damaged  = true;
    }

    set_sprite(player->damage, get_damage_sprite(player->ship_type, player->damage_type), player->entity->sprite_size, 1);
    
    player->invincibility_timer = 1.5f;
    player->is_invincible       = true;
}

void on_create(Player* player) {
    set_collider(player->entity, 0.5f);

    player->left_thrust  = create_entity(ENTITY_TYPE_NONE, player->entity);
    player->right_thrust = create_entity(ENTITY_TYPE_NONE, player->entity);
    
    player->left_thrust->position = make_vector2(-0.3f, -0.5f);
    set_sprite(player->left_thrust, &sprite_thrust, 0.5f, -1);

    player->right_thrust->position = make_vector2(0.3f, -0.5f);
    set_sprite(player->right_thrust, &sprite_thrust, 0.5f, -1);

    player->left_thrust->is_visible  = false;
    player->right_thrust->is_visible = false;

    player->damage = create_entity(ENTITY_TYPE_NONE, player->entity);
    play_sound(&sound_spawn);
}

void on_destroy(Player* player) {
    play_sound(get_kill_sound());
}

void on_update(Player* player) {
    Vector2 acceleration;

    if (input.gamepad_left_y > 0.0f) {
        acceleration = get_direction(player->entity->orientation) * 10.0f * input.gamepad_left_y;
    }

    if (input.key_w.held) {
        acceleration = get_direction(player->entity->orientation) * 10.0f;
    }

    player->entity->position += (player->velocity * timers.delta) + (0.5f * acceleration * square(timers.delta));

    player->velocity += acceleration * timers.delta;
    player->velocity -= player->velocity * 0.5f * timers.delta;

    if (input.key_w.down) {
        player->left_thrust->is_visible  = true;
        player->right_thrust->is_visible = true;
    }

    if (input.key_w.up) {
        player->left_thrust->is_visible  = false;
        player->right_thrust->is_visible = false;
    }

    if (input.gamepad_right_x) {
        f32 desired_orientation = player->entity->orientation - (3500.0f * input.gamepad_right_x * timers.delta);
        player->desired_direction = get_direction(desired_orientation);
    }

    if (input.mouse_x != player->last_mouse_x || input.mouse_y != player->last_mouse_y) {
        player->desired_direction = normalize(get_world_position(input.mouse_x, input.mouse_y) - player->entity->position);
        
        player->last_mouse_x = input.mouse_x;
        player->last_mouse_y = input.mouse_y;
    }

    Vector2 current_direction = get_direction(player->entity->orientation);
    Vector2 new_direction     = lerp(current_direction, 12.5f * timers.delta, player->desired_direction);

    player->entity->orientation = get_angle(new_direction);

    if (input.mouse_left.down || input.gamepad_right_trigger.down) {
        Laser* laser = create_entity(ENTITY_TYPE_LASER)->laser;
        init_laser(laser, LASER_COLOR_BLUE, player->entity, player->entity->orientation);

        play_sound(get_laser_sound());
    }

    if (player->is_invincible && (player->invincibility_timer -= timers.delta) <= 0.0f) {
        player->is_invincible = false;
    }
}

void on_collision(Player* player, Entity* them) {
    switch (them->type) {
        case ENTITY_TYPE_ASTEROID: {
            if (!player->is_invincible) {
                damage_player(player);

                spawn_children(them->asteroid);
                destroy_entity(them);
            }

            break;
        }
    }
}

#endif