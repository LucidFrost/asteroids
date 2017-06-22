struct Player {
    
};

void init_player(Entity* entity) {
    entity->sprite = &ship_sprite;
}

void update_player(Entity* entity) {
    if (input.key_w.held) {
        entity->position += get_direction(entity->orientation) * 5.0f * time.delta;
    }

    if (input.key_s.held) {
        entity->position -= get_direction(entity->orientation) * 5.0f * time.delta;
    }

    if (input.key_a.held) {
        entity->orientation += 100.0f * time.delta;
    }

    if (input.key_d.held) {
        entity->orientation -= 100.0f * time.delta;
    }
}