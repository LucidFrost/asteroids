// @todo:
//  - The player can send the ship into hyperspace, causing it to disappear and reappear 
//    in a random location on the screen, at the risk of self-destructing or appearing on top 
//    of an asteroid.
//  - Respawn invincibility
//

void add_score(Player* player, u32 score) {
    player->score += score;
    player->score_since_last_life += score;

    if (player->score_since_last_life >= 10000) {
        player->score_since_last_life = 0;
        player->lives += 1;
    }
}

void spawn_player(Player* player) {
    assert(player->is_dead);
    assert(player->lives > 0);

    player->entity->position    = make_vector2();
    player->entity->orientation = 0.0f;
    player->entity->is_visible  = true;
    player->velocity            = make_vector2();
    player->desired_direction   = make_vector2();
    player->is_dead             = false;
    player->has_shield          = true;
    player->shield_timer        = 2.5f;
    player->shield->is_visible  = true;

    play_sound(&spawn_sound);
}

void kill_player(Player* player) {
    assert(!player->is_dead);
    assert(!player->has_shield);

    player->entity->is_visible = false;

    player->lives -= 1;
    player->is_dead = true;

    player->left_thrust->is_visible  = false;
    player->right_thrust->is_visible = false;

    play_sound(&kill_01_sound);
}

void destroy_shield(Player* player) {
    player->has_shield         = false;
    player->shield->is_visible = false;
}

void on_create(Player* player) {
    player->entity->sprite          = &ship_sprite;
    player->entity->has_collider    = true;
    player->entity->collider_radius = 0.5f;

    player->left_thrust = create_entity(Entity_Type::NONE, player->entity);
    
    player->left_thrust->position     = make_vector2(-0.3f, -0.5f);
    player->left_thrust->sprite       = &thrust_sprite;
    player->left_thrust->sprite_size  = 0.5f;
    player->left_thrust->sprite_order = -1;

    player->right_thrust = create_entity(Entity_Type::NONE, player->entity);

    player->right_thrust->position     = make_vector2(0.3f, -0.5f);
    player->right_thrust->sprite       = &thrust_sprite;
    player->right_thrust->sprite_size  = 0.5f;
    player->right_thrust->sprite_order = -1;

    player->shield = create_entity(Entity_Type::NONE, player->entity);

    player->shield->sprite       = &shield_sprite;
    player->shield->sprite_size  = 1.75f;
    player->shield->sprite_order = 1;
}

void on_destroy(Player* player) {
    destroy_entity(player->left_thrust);
    destroy_entity(player->right_thrust);

    destroy_entity(player->shield);
}

void on_update(Player* player) {
    if (player->is_dead) {
        if (player->lives && input.key_space.down) {
            spawn_player(player);
        }
    }
    else {
        Vector2 acceleration;

        if (input.gamepad_left_y > 0.0f) {
            acceleration = get_direction(player->entity->orientation) * 10.0f * input.gamepad_left_y;
        }

        if (input.key_w.held) {
            acceleration = get_direction(player->entity->orientation) * 10.0f;
        }

        player->entity->position += (player->velocity * time.delta) + (0.5f * acceleration * square(time.delta));

        player->velocity += acceleration * time.delta;
        player->velocity -= player->velocity * 0.5f * time.delta;

        if (input.key_w.down) {
            player->left_thrust->is_visible  = true;
            player->right_thrust->is_visible = true;
        }

        if (input.key_w.up) {
            player->left_thrust->is_visible  = false;
            player->right_thrust->is_visible = false;
        }

        if (input.gamepad_right_x) {
            f32 desired_orientation = player->entity->orientation - (3500.0f * input.gamepad_right_x * time.delta);
            player->desired_direction = get_direction(desired_orientation);
        }

        if (input.mouse_x != player->last_mouse_x || input.mouse_y != player->last_mouse_y) {
            f32 normalized_x = ((2.0f * input.mouse_x) / window_width) - 1.0f;
            f32 normalized_y = 1.0f - ((2.0f * input.mouse_y) / window_height);

            Vector2 mouse_position = make_inverse_matrix(world_projection) * make_vector2(normalized_x, normalized_y);
            player->desired_direction = normalize(mouse_position - player->entity->position);
            
            player->last_mouse_x = input.mouse_x;
            player->last_mouse_y = input.mouse_y;
        }

        Vector2 current_direction = get_direction(player->entity->orientation);
        Vector2 new_direction     = lerp(current_direction, 12.5f * time.delta, player->desired_direction);

        player->entity->orientation = get_angle(new_direction);

        if (input.mouse_left.down || input.gamepad_right_trigger.down) {
            Laser* laser = create_entity(Entity_Type::LASER)->laser;

            laser->shooter             = player->entity;
            laser->entity->sprite      = &laser_blue_sprite;
            laser->entity->position    = player->entity->position + (get_direction(player->entity->orientation) * 0.75f);
            laser->entity->orientation = player->entity->orientation;
        }

        if (player->has_shield) {
            if ((player->shield_timer -= time.delta) <= 0.0f) {
                destroy_shield(player);
            }
        }
    }
}

void on_collision(Player* player, Entity* them) {
    switch (them->type) {
        case Entity_Type::ASTEROID: {
            if (!player->is_dead) {
                if (player->has_shield) {
                    destroy_shield(player);
                }
                else {
                    kill_player(player);
                }

                destroy_entity(them);
            }

            break;
        }
    }
}