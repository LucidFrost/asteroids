struct Entity;
struct Player;

enum struct Entity_Type {
    NONE,
    PLAYER
};

char* to_string(Entity_Type entity_type) {
    switch (entity_type) {
        case Entity_Type::NONE:   return "NONE";
        case Entity_Type::PLAYER: return "PLAYER";
    }

    return "INVALID";
}

typedef void Entity_Update(Entity* entity);

struct Entity {
    int id = -1;

    Entity_Type    type   = Entity_Type::NONE;
    Entity_Update* update = NULL;
    
    Vector2 position;
    float   orientation = 0.0f;
    float   scale       = 1.0f;

    Sprite* sprite = NULL;

    union {
        void*   derived = NULL;
        Player* player;
    };
};

Entity* make_entity(Entity_Type type);

#include "player.cpp"

Entity entity_buffer[32];
Player player_buffer[1];

bool entity_buffer_mask[count_of(entity_buffer)];
bool player_buffer_mask[count_of(player_buffer)];

int next_entity_id = 0;

Entity* make_entity(Entity_Type type) {
    Entity* entity = NULL;
    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (entity_buffer_mask[i]) continue;
        entity_buffer_mask[i] = true;

        entity = new(&entity_buffer[i]) Entity;
        break;
    }

    assert(entity);

    entity->id   = next_entity_id;
    entity->type = type;

    switch (entity->type) {
        case Entity_Type::PLAYER: {
            entity->update = update_player;

            for (int i = 0; i < count_of(player_buffer); i++) {
                if (player_buffer_mask[i]) continue;
                player_buffer_mask[i] = true;

                entity->player = new(&player_buffer[i]) Player;
                break;
            }

            assert(entity->player);
            init_player(entity);

            break;
        }
        default: {
            printf("Failed to make entity, unhandled entity type '%s' (%i) specified\n", to_string(entity->type), entity->type);
            assert(false);

            break;
        }
    }

    printf("Made entity type '%s' (id: %i)\n", to_string(entity->type), entity->id);
    next_entity_id += 1;
    
    return entity;
}

const float WORLD_PROJECTION_HEIGHT = 15.0f;
const float WORLD_PROJECTION_WIDTH  = WORLD_PROJECTION_HEIGHT * ((float) WINDOW_WIDTH / (float) WINDOW_HEIGHT);

Matrix4 world_projection;
Matrix4 gui_projection;

void init_asteroids() {
    world_projection = make_orthographic_matrix(
        -WORLD_PROJECTION_WIDTH  / 2.0f,
         WORLD_PROJECTION_WIDTH  / 2.0f,
         WORLD_PROJECTION_HEIGHT / 2.0f,
        -WORLD_PROJECTION_HEIGHT / 2.0f);

    gui_projection = make_orthographic_matrix(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);

    make_entity(Entity_Type::PLAYER);
}

void update_asteroids() {
    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i]) continue;

        Entity* entity = &entity_buffer[i];
        entity->update(entity);
    }

    set_projection(&world_projection);

    for (int x = 0; x < 6; x++) {
        for (int y = 0; y < 3; y++) {
            Vector2 position = make_vector2(-WORLD_PROJECTION_WIDTH / 2.0f, -WORLD_PROJECTION_HEIGHT / 2.0f);
            position += make_vector2(x, y) * 5.0f;

            Matrix4 transform = make_transform_matrix(position, 0.0f, 5.0f);
            set_transform(&transform);

            draw_sprite(&background_sprite, false);
        }
    }

    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i]) continue;

        Entity* entity = &entity_buffer[i];
        if (entity->sprite) {
            Matrix4 transform = make_transform_matrix(entity->position, entity->orientation);
            set_transform(&transform);

            draw_sprite(entity->sprite);
        }
    }

    set_projection(&gui_projection);

    // @todo: move the debug gui drawing out of asteroids and into main?
    
    Matrix4 transform = make_transform_matrix(make_vector2(16.0f, WINDOW_HEIGHT - font_height - 16.0f));
    set_transform(&transform);
    
    draw_text("%.2f, %.2f, %i", time.now, time.delta * 1000.0f, (int) (1.0f / time.delta));
}