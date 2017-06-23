struct Player {
    Vector2 velocity;
    Vector2 desired_direction;

    int last_mouse_x = 0;
    int last_mouse_y = 0;
};

void create_player(Entity* entity) {
    entity->sprite = &ship_sprite;
}

void destroy_player(Entity* entity) {

}

void update_player(Entity* entity) {
    Vector2 acceleration;

    if (input.key_w.held) acceleration += get_direction(entity->orientation) * 5.0f;
    if (input.key_s.held) acceleration -= get_direction(entity->orientation);

    entity->position += (entity->player->velocity * time.delta) + (0.5f * acceleration * square(time.delta));
    entity->player->velocity += acceleration * time.delta;

    if (input.mouse_x != entity->player->last_mouse_x || input.mouse_y != entity->player->last_mouse_y) {
        float normalized_x = ((2.0f * input.mouse_x) / WINDOW_WIDTH) - 1.0f;
        float normalized_y = 1.0f - ((2.0f * input.mouse_y) / WINDOW_HEIGHT);

        Vector2 mouse_position = make_inverse_matrix(world_projection) * make_vector2(normalized_x, normalized_y);
        entity->player->desired_direction = normalize(mouse_position - entity->position);
        
        entity->player->last_mouse_x = input.mouse_x;
        entity->player->last_mouse_y = input.mouse_y;
    }

    Vector2 current_direction = get_direction(entity->orientation);
    Vector2 new_direction     = lerp(current_direction, 10.0f * time.delta, entity->player->desired_direction);

    entity->orientation = get_angle(new_direction);

    if (input.mouse_left.down) {
        Entity* laser_entity = create_entity(Entity_Type::LASER);
        laser_entity->laser->shooter_id = entity->id;

        laser_entity->position    = entity->position;
        laser_entity->orientation = entity->orientation;
    }
}