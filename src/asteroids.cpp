const float WORLD_HEIGHT = 15.0f;
const float WORLD_WIDTH  = WORLD_HEIGHT * ((float) WINDOW_WIDTH / (float) WINDOW_HEIGHT);

const float WORLD_LEFT   = -WORLD_WIDTH  / 2.0f;
const float WORLD_RIGHT  =  WORLD_WIDTH  / 2.0f;
const float WORLD_TOP    =  WORLD_HEIGHT / 2.0f;
const float WORLD_BOTTOM = -WORLD_HEIGHT / 2.0f;

Matrix4 world_projection;
Matrix4 gui_projection;

struct Entity;
struct Player;
struct Laser;

enum struct Entity_Type {
    NONE,
    PLAYER,
    LASER
};

char* to_string(Entity_Type entity_type) {
    switch (entity_type) {
        case Entity_Type::NONE:   return "NONE";
        case Entity_Type::PLAYER: return "PLAYER";
        case Entity_Type::LASER:  return "LASER";
    }

    return "INVALID";
}

typedef void Entity_Create(Entity* entity);
typedef void Entity_Destroy(Entity* entity);
typedef void Entity_Update(Entity* entity);

struct Entity {
    int id = -1;
    Entity_Type type = Entity_Type::NONE;

    Entity_Create*  create  = NULL;
    Entity_Destroy* destroy = NULL;
    Entity_Update*  update  = NULL;
    
    Vector2 position;
    float   orientation = 0.0f;
    float   scale       = 1.0f;

    Sprite* sprite = NULL;

    union {
        void*   derived = NULL;
        Player* player;
        Laser*  laser;
    };
};

Entity* create_entity(Entity_Type type);
void    destroy_entity(Entity* entity);

#include "laser.cpp"
#include "player.cpp"

#define entity_storage(Type, type, count)   \
    Type type##_buffer[count];              \
    bool type##_buffer_mask[count]

entity_storage(Entity, entity, 512);
entity_storage(Player, player, 1);
entity_storage(Laser, laser, 32);

#undef entity_storage

int next_entity_id = 0;

Entity* create_entity(Entity_Type type) {
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

    #define create_entity_case(TYPE, Type, type)                    \
        case Entity_Type::TYPE: {                                   \
            entity->create  = create_##type;                        \
            entity->destroy = destroy_##type;                       \
            entity->update  = update_##type;                        \
                                                                    \
            for (int i = 0; i < count_of(type##_buffer); i++) {     \
                if (type##_buffer_mask[i]) continue;                \
                type##_buffer_mask[i] = true;                       \
                                                                    \
                entity->type = new(&type##_buffer[i]) Type;         \
                break;                                              \
            }                                                       \
                                                                    \
            break;                                                  \
        }

    switch (entity->type) {
        create_entity_case(PLAYER, Player, player);
        create_entity_case(LASER, Laser, laser);

        default: {
            printf("Failed to create entity, unhandled entity type '%s' (%i) specified\n", to_string(entity->type), entity->type);
            assert(false);

            break;
        }
    }

    #undef create_entity_case

    assert(entity->create);
    assert(entity->destroy);
    assert(entity->update);
    assert(entity->derived);

    printf("Created entity type '%s' (id: %i)\n", to_string(entity->type), entity->id);
    next_entity_id += 1;

    entity->create(entity);

    return entity;
}

void destroy_entity(Entity* entity) {
    int entity_index = (int) (entity - entity_buffer);

    assert(entity_index < count_of(entity_buffer));
    assert(entity_buffer_mask[entity_index]);

    entity->destroy(entity);

    #define destroy_entity_case(TYPE, type)                             \
        case Entity_Type::TYPE: {                                       \
            int type##_index = (int) (entity->type - type##_buffer);    \
                                                                        \
            assert(type##_index < count_of(type##_buffer));             \
            assert(type##_buffer_mask[type##_index]);                   \
                                                                        \
            type##_buffer_mask[type##_index] = false;                   \
            break;                                                      \
        }

    switch (entity->type) {
        destroy_entity_case(PLAYER, player);
        destroy_entity_case(LASER, laser);

        default: {
            printf("Failed to destroy entity, unhandled entity type '%s' (%i) specified\n", to_string(entity->type), entity->type);
            break;
        }
    }

    #undef destroy_entity_case

    printf("Destroyed entity type '%s', (id: %i)\n", to_string(entity->type), entity->id);
    entity_buffer_mask[entity_index] = false;
}

void draw_entity_at(Entity* entity, Vector2 position, bool show_collider = true) {
    Matrix4 transform = make_transform_matrix(position, entity->orientation);
    set_transform(&transform);

    draw_sprite(entity->sprite, entity->sprite->aspect, 1.0f);

    // if (show_collider) {
    //     draw_rectangle(entity->sprite->aspect, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, true, false);
    // }
}

void init_asteroids() {
    world_projection = make_orthographic_matrix(WORLD_LEFT, WORLD_RIGHT, WORLD_TOP, WORLD_BOTTOM);
    gui_projection = make_orthographic_matrix(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);

    create_entity(Entity_Type::PLAYER);
}

void update_asteroids() {
    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i]) continue;

        Entity* entity = &entity_buffer[i];
        entity->update(entity);

        if (entity->position.x < WORLD_LEFT)   entity->position.x = WORLD_RIGHT;
        if (entity->position.x > WORLD_RIGHT)  entity->position.x = WORLD_LEFT;
        if (entity->position.y < WORLD_BOTTOM) entity->position.y = WORLD_TOP;
        if (entity->position.y > WORLD_TOP)    entity->position.y = WORLD_BOTTOM;
    }

    set_projection(&world_projection);

    for (int x = 0; x < 6; x++) {
        for (int y = 0; y < 3; y++) {
            Vector2 position = make_vector2(WORLD_LEFT, WORLD_BOTTOM);
            position += make_vector2(x, y) * 5.0f;

            Matrix4 transform = make_transform_matrix(position, 0.0f, 5.0f);
            set_transform(&transform);

            draw_sprite(&background_sprite, 1.0f, 1.0f, false);
        }
    }

    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i]) continue;

        Entity* entity = &entity_buffer[i];
        if (entity->sprite) {
            draw_entity_at(entity, entity->position);

            float bounds = entity->sprite->width > entity->sprite->height ? entity->sprite->width : entity->sprite->height;

            if (entity->position.x - WORLD_LEFT <= bounds) {
                Vector2 position = make_vector2(WORLD_RIGHT + (entity->position.x - WORLD_LEFT), entity->position.y);
                draw_entity_at(entity, position);
            }

            if (WORLD_RIGHT - entity->position.x <= bounds) {
                Vector2 position = make_vector2(WORLD_LEFT - (WORLD_RIGHT - entity->position.x), entity->position.y);
                draw_entity_at(entity, position);
            }

            if (entity->position.y - WORLD_BOTTOM <= bounds) {
                Vector2 position = make_vector2(entity->position.x, WORLD_TOP + (entity->position.y - WORLD_BOTTOM));
                draw_entity_at(entity, position);
            }

            if (WORLD_TOP - entity->position.y <= bounds) {
                Vector2 position = make_vector2(entity->position.x, WORLD_BOTTOM - (WORLD_TOP - entity->position.y));
                draw_entity_at(entity, position);
            }
        }
    }
}