struct Laser {
    int   player_id = -1;
    float lifetime  = 1.0f;
};

void laser_on_create(Entity* entity) {
    entity->has_collider    = true;
    entity->collider_radius = 0.1f;

    entity->sprite        = &laser_sprite;
    entity->sprite_size   = 0.75f;
    entity->sprite_offset = make_vector2(0.0f, -0.3f);
}

void laser_on_destroy(Entity* entity) {

}

void laser_on_update(Entity* entity) {
    if ((entity->laser->lifetime -= time.delta) <= 0.0f) {
        destroy_entity(entity);
    }
    else {
        entity->position += get_direction(entity->orientation) * 15.0f * time.delta;
    }
}

void laser_on_collision(Entity* us, Entity* them) {
    void add_score(Entity* entity, int score);

    if (them->type == Entity_Type::ASTEROID) {
        Entity* player = find_entity(us->laser->player_id);
        add_score(player, them->asteroid->score);

        destroy_entity(us);
        destroy_entity(them);
    }
}