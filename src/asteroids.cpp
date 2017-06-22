struct Entity;
struct Player;

enum struct Entity_Type {
    NONE,
    PLAYER
};

typedef void Entity_Update(Entity* entity);

struct Entity {
    int id = -1;

    Entity_Type    type   = Entity_Type::NONE;
    Entity_Update* update = NULL;
    
    Vector2 position;
    float   orientation = 0.0f;

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
            assert(false);
            break;
        }
    }

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
    set_projection(&world_projection);

    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i]) continue;

        Entity* entity = &entity_buffer[i];
        entity->update(entity);

        Matrix4 transform = make_transform_matrix(entity->position, entity->orientation);
        set_transform(&transform);
        
        draw_rectangle(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}