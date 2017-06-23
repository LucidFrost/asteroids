struct Laser {
    int shooter_id = -1;
    float lifetime = 1.5f;
};

void create_laser(Entity* entity) {
    entity->sprite = &laser_sprite;
}

void destroy_laser(Entity* entity) {

}

void update_laser(Entity* entity) {
    if ((entity->laser->lifetime -= time.delta) <= 0.0f) {
        destroy_entity(entity);
    }
    else {
        entity->position += get_direction(entity->orientation) * 15.0f * time.delta;
    }
}