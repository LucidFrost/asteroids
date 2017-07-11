struct Particle {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;

    f32 scale = 1.0f;
    f32 opacity = 1.0f;

    Sprite* sprite = null;
    f32     size = 0.0f;

    f32 lifetime = 0.0f;
    bool is_alive = false;
};

Particle particles[1024];
u32 next_particle = 0;

void spawn_particles(Vector2 position, Vector2 velocity, f32 scale) {
    u32 amount = get_random_between(5, 10);
    for (u32 i = 0; i < amount; i++) {
        Particle* particle = &particles[next_particle];
        next_particle += 1;

        if (next_particle >= count_of(particles)) {
            next_particle = 0;
        }

        particle->position = position;
        particle->velocity = make_vector2(0.0f, 0.0f);
        
        particle->scale = scale;
        particle->opacity = 1.0f;

        particle->position.x += get_random_between(-0.25f, 0.25f);
        particle->position.y += get_random_between(-0.25f, 0.25f);

        particle->velocity.x += get_random_between(-0.5f, 0.5f);
        particle->velocity.y += get_random_between(-0.5f, 0.5f);
        
        particle->acceleration.x = get_random_between(-0.25f, 0.25f);
        particle->acceleration.y = get_random_between(-0.25f, 0.25f);

        u32 index = get_random_out_of(9);
        
        particle->sprite = &sprite_smoke[index];
        particle->size   = index / 10.0f;

        particle->lifetime = get_random_between(0.25f, 0.75f);
        particle->is_alive = true;
    }
}

void update_particles() {
    for (u32 i = 0; i < count_of(particles); i++) {
        Particle* particle = &particles[i];
        if (!particle->is_alive) continue;

        if ((particle->lifetime -= timers.delta) <= 0.0f) {
            particle->is_alive = false;
            continue;
        }

        particle->position += (0.5f * square(timers.delta) * particle->acceleration) + (timers.delta * particle->velocity);
        particle->velocity += timers.delta * particle->acceleration;

        particle->scale = lerp(particle->scale, timers.delta, 0.0f);
        particle->opacity = lerp(particle->opacity, timers.delta, 0.0f);
    }
}

void draw_particles() {
    for (u32 i = 0; i < count_of(particles); i++) {
        Particle* particle = &particles[i];
        if (!particle->is_alive) continue;

        set_transform(make_transform_matrix(particle->position, 0.0f, particle->scale));
        draw_sprite(particle->sprite, particle->size, particle->opacity);
    }
}