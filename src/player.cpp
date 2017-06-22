struct Player {
    
};

void init_player(Entity* entity) {
    printf("Init player (id: %i)\n", entity->id);
}

void update_player(Entity* entity) {
    if (input.key_w.held) {
        entity->position += get_direction(entity->orientation) * time.frame_delta;
    }

    if (input.key_a.held) {
        entity->orientation += 100.0f * time.frame_delta;
    }

    if (input.key_d.held) {
        entity->orientation -= 100.0f * time.frame_delta;
    }
}