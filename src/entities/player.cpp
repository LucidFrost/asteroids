#if AS_HEADER
    struct Player {
        Entity* entity = null;

        Entity* left_thrust  = null;
        Entity* right_thrust = null;
        Entity* shield       = null;

        Vector2 velocity;
        Vector2 desired_direction;

        bool has_shield   = false;
        f32  shield_timer = 0.0f;

        i32 last_mouse_x = 0;
        i32 last_mouse_y = 0;

        u32 score = 0;
        u32 score_since_last_life = 0;
    };

    void on_create(Player* player);
    void on_destroy(Player* player);
    void on_update(Player* player);
    void on_collision(Player* player, Entity* them);
#else
    void add_score(u32 score);
    void kill_player();

    void destroy_shield(Player* player) {
        player->has_shield         = false;
        player->shield->is_visible = false;
    }

    void on_create(Player* player) {
        player->entity->sprite          = get_ship_sprite(SHIP_COLOR_BLUE, SHIP_TYPE_1);
        player->entity->is_visible      = true;
        player->entity->has_collider    = true;
        player->entity->collider_radius = 0.5f;

        player->left_thrust = create_entity(ENTITY_TYPE_NONE, player->entity);
        
        player->left_thrust->position     = make_vector2(-0.3f, -0.5f);
        player->left_thrust->sprite       = &sprite_thrust;
        player->left_thrust->sprite_size  = 0.5f;
        player->left_thrust->sprite_order = -1;

        player->right_thrust = create_entity(ENTITY_TYPE_NONE, player->entity);

        player->right_thrust->position     = make_vector2(0.3f, -0.5f);
        player->right_thrust->sprite       = &sprite_thrust;
        player->right_thrust->sprite_size  = 0.5f;
        player->right_thrust->sprite_order = -1;

        player->shield = create_entity(ENTITY_TYPE_NONE, player->entity);

        player->shield->sprite       = &sprite_shield;
        player->shield->sprite_size  = 1.75f;
        player->shield->sprite_order = 1;

        player->has_shield          = true;
        player->shield_timer        = 2.5f;
        player->shield->is_visible  = true;

        play_sound(&spawn_sound);
    }

    void on_destroy(Player* player) {
        destroy_entity(player->left_thrust);
        destroy_entity(player->right_thrust);

        destroy_entity(player->shield);
        play_sound(&kill_01_sound);
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
            Vector2 world_position = unproject(input.mouse_x, input.mouse_y, platform.window_width, platform.window_height, world_projection);
            player->desired_direction = normalize(world_position - player->entity->position);
            
            player->last_mouse_x = input.mouse_x;
            player->last_mouse_y = input.mouse_y;
        }

        Vector2 current_direction = get_direction(player->entity->orientation);
        Vector2 new_direction     = lerp(current_direction, 12.5f * timers.delta, player->desired_direction);

        player->entity->orientation = get_angle(new_direction);

        if (input.mouse_left.down || input.gamepad_right_trigger.down) {
            Laser* laser = create_entity(ENTITY_TYPE_LASER)->laser;

            laser->shooter_id          = player->entity->id;
            laser->entity->sprite      = get_laser_sprite(LASER_COLOR_BLUE);
            laser->entity->position    = player->entity->position + (get_direction(player->entity->orientation) * 0.75f);
            laser->entity->orientation = player->entity->orientation;
        }

        if (player->has_shield) {
            if ((player->shield_timer -= timers.delta) <= 0.0f) {
                destroy_shield(player);
            }
        }
    }

    void on_collision(Player* player, Entity* them) {
        switch (them->type) {
            case ENTITY_TYPE_ASTEROID: {
                kill_player();

                spawn_children(them->asteroid);
                destroy_entity(them);

                break;
            }
        }
    }
#endif