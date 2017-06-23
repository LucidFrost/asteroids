// @todo:
//  - The player can send the ship into hyperspace, causing it to disappear and reappear 
//    in a random location on the screen, at the risk of self-destructing or appearing on top 
//    of an asteroid.
//

struct Player {
    int left_thrust_id  = -1;
    int right_thrust_id = -1;

    Vector2 velocity;
    Vector2 desired_direction;

    int last_mouse_x = 0;
    int last_mouse_y = 0;

    int score = 0;
};

void add_score(Entity* entity, int score) {
    entity->player->score += score;
}

void player_on_create(Entity* entity) {
    entity->has_collider    = true;
    entity->collider_radius = 0.5f;

    entity->sprite = &ship_sprite;

    Entity* left_thrust = create_entity(Entity_Type::NONE, entity);
    left_thrust->position = make_vector2(-0.3f, -0.5f);

    left_thrust->sprite      = &thrust_sprite;
    left_thrust->sprite_size = 0.5f;
    left_thrust->is_visible  = false;

    Entity* right_thrust = create_entity(Entity_Type::NONE, entity);
    right_thrust->position = make_vector2(0.3f, -0.5f);

    right_thrust->sprite      = &thrust_sprite;
    right_thrust->sprite_size = 0.5f;
    right_thrust->is_visible  = false;

    entity->player->left_thrust_id  = left_thrust->id;
    entity->player->right_thrust_id = right_thrust->id;
}

void player_on_destroy(Entity* entity) {
    Entity* left_thrust  = find_entity(entity->player->left_thrust_id);
    Entity* right_thrust = find_entity(entity->player->right_thrust_id);
    
    destroy_entity(left_thrust);
    destroy_entity(right_thrust);
}

void player_on_update(Entity* entity) {
    Vector2 acceleration;

    if (input.key_w.held) acceleration += get_direction(entity->orientation) * 15.0f;
    if (input.key_s.held) acceleration -= get_direction(entity->orientation);

    entity->position += (entity->player->velocity * time.delta) + (0.5f * acceleration * square(time.delta));

    entity->player->velocity += acceleration * time.delta;
    entity->player->velocity -= entity->player->velocity * 0.5f * time.delta;

    Entity* left_thrust  = find_entity(entity->player->left_thrust_id);
    Entity* right_thrust = find_entity(entity->player->right_thrust_id);

    if (input.key_w.down) {
        left_thrust->is_visible  = true;
        right_thrust->is_visible = true;
    }

    if (input.key_w.up) {
        left_thrust->is_visible  = false;
        right_thrust->is_visible = false;
    }

    if (input.mouse_x != entity->player->last_mouse_x || input.mouse_y != entity->player->last_mouse_y) {
        float normalized_x = ((2.0f * input.mouse_x) / WINDOW_WIDTH) - 1.0f;
        float normalized_y = 1.0f - ((2.0f * input.mouse_y) / WINDOW_HEIGHT);

        Vector2 mouse_position = make_inverse_matrix(world_projection) * make_vector2(normalized_x, normalized_y);
        entity->player->desired_direction = normalize(mouse_position - entity->position);
        
        entity->player->last_mouse_x = input.mouse_x;
        entity->player->last_mouse_y = input.mouse_y;
    }

    Vector2 current_direction = get_direction(entity->orientation);
    Vector2 new_direction     = lerp(current_direction, 12.5f * time.delta, entity->player->desired_direction);

    entity->orientation = get_angle(new_direction);

    if (input.mouse_left.down) {
        Entity* laser_entity = create_entity(Entity_Type::LASER);
        laser_entity->laser->player_id = entity->id;

        laser_entity->position    = entity->position + (get_direction(entity->orientation) * 0.75f);
        laser_entity->orientation = entity->orientation;
    }
}

void player_on_collision(Entity* us, Entity* them) {
    if (them->type == Entity_Type::ASTEROID) {
        us->position    = make_vector2();
        us->orientation = 0.0f;
        
        us->player->velocity          = make_vector2();
        us->player->desired_direction = make_vector2();

        destroy_entity(them);
    }
}