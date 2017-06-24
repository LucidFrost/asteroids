const float WORLD_HEIGHT = 15.0f;
const float WORLD_WIDTH  = WORLD_HEIGHT * ((float) WINDOW_WIDTH / (float) WINDOW_HEIGHT);

const float WORLD_LEFT   = -WORLD_WIDTH  / 2.0f;
const float WORLD_RIGHT  =  WORLD_WIDTH  / 2.0f;
const float WORLD_TOP    =  WORLD_HEIGHT / 2.0f;
const float WORLD_BOTTOM = -WORLD_HEIGHT / 2.0f;

Matrix4 world_projection;
Matrix4 gui_projection;

Playing_Sound* playing_music;
float music_volume;

bool is_showing_menu;

struct Entity;
struct Player;
struct Laser;
struct Asteroid;
struct Enemy;

enum struct Entity_Type {
    NONE,
    PLAYER,
    LASER,
    ASTEROID,
    ENEMY
};

char* to_string(Entity_Type entity_type) {
    #define case(type) case Entity_Type::type: return #type

    switch (entity_type) {
        case(NONE);
        case(PLAYER);
        case(LASER);
        case(ASTEROID);
        case(ENEMY);
    }

    #undef case

    return "INVALID";
}

typedef void Entity_On_Create(Entity* entity);
typedef void Entity_On_Destroy(Entity* entity);
typedef void Entity_On_Update(Entity* entity);
typedef void Entity_On_Collision(Entity* us, Entity* them);

struct Entity {
    int id = -1;
    Entity_Type type = Entity_Type::NONE;
    
    Entity* parent  = NULL;
    Entity* child   = NULL;
    Entity* sibling = NULL;

    Matrix4 transform;

    Vector2 position;
    float   orientation = 0.0f;
    float   scale       = 1.0f;

    bool  has_collider    = false;
    float collider_radius = 0.0f;

    Sprite* sprite        = NULL;
    float   sprite_size   = 1.0f;
    Vector2 sprite_offset;
    bool    is_visible    = true;

    Entity_On_Create*    on_create    = NULL;
    Entity_On_Destroy*   on_destroy   = NULL;
    Entity_On_Update*    on_update    = NULL;
    Entity_On_Collision* on_collision = NULL;

    union {
        void*     derived = NULL;
        Player*   player;
        Laser*    laser;
        Asteroid* asteroid;
        Enemy*    enemy;
    };
};

Entity root_entity;

Entity* the_player;
Entity* the_enemy;

Entity* create_entity(Entity_Type type, Entity* parent = &root_entity);
void    destroy_entity(Entity* entity);
Entity* find_entity(int id);

#include "asteroid.cpp"
#include "player.cpp"
#include "enemy.cpp"
#include "laser.cpp"

#define entity_storage(Type, type, count)   \
    Type type##_buffer[count];              \
    bool type##_buffer_mask[count]

entity_storage(Entity, entity, 512);
entity_storage(Player, player, 1);
entity_storage(Laser, laser, 32);
entity_storage(Asteroid, asteroid, 32);
entity_storage(Enemy, enemy, 1);

#undef entity_storage

int next_entity_id  = 0;
int active_entities = 0;

Entity* create_entity(Entity_Type type, Entity* parent) {
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

    if (parent != &root_entity) {
        int parent_index = (int) (parent - entity_buffer);

        assert(parent_index < count_of(entity_buffer));
        assert(entity_buffer_mask[parent_index]);
    }

    entity->parent = parent;

    if (entity->parent->child) {
        Entity* child = entity->parent->child;
        while (child->sibling) {
            child = child->sibling;
        }

        child->sibling = entity;
    }
    else {
        entity->parent->child = entity;
    }

    #define case(TYPE, Type, type)                                  \
        case Entity_Type::TYPE: {                                   \
            entity->on_create    = type##_on_create;                \
            entity->on_destroy   = type##_on_destroy;               \
            entity->on_update    = type##_on_update;                \
            entity->on_collision = type##_on_collision;             \
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
        case Entity_Type::NONE: {
            break;
        }

        case(PLAYER,   Player,   player);
        case(LASER,    Laser,    laser);
        case(ASTEROID, Asteroid, asteroid);
        case(ENEMY,    Enemy,    enemy);

        default: {
            printf("Failed to create entity, unhandled entity type '%s' (%i) specified\n", to_string(entity->type), entity->type);
            assert(false);

            break;
        }
    }

    #undef case

    next_entity_id  += 1;
    active_entities += 1;

    if (entity->on_create) {
        entity->on_create(entity);
    }

    return entity;
}

void destroy_entity(Entity* entity) {
    int entity_index = (int) (entity - entity_buffer);

    assert(entity_index < count_of(entity_buffer));
    assert(entity_buffer_mask[entity_index]);

    if (entity->on_destroy) {
        entity->on_destroy(entity);
    }

    #define case(TYPE, type)                             \
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
        case Entity_Type::NONE: {
            break;
        }

        case(PLAYER,   player);
        case(LASER,    laser);
        case(ASTEROID, asteroid);
        case(ENEMY,    enemy);

        default: {
            printf("Failed to destroy entity, unhandled entity type '%s' (%i) specified\n", to_string(entity->type), entity->type);
            break;
        }
    }

    #undef case

    if (entity->parent->child == entity) {
        entity->parent->child = entity->sibling;
    }
    else {
        Entity* child = entity->parent->child;
        while (child->sibling != entity) {
            child = child->sibling;
        }

        child->sibling = entity->sibling;
    }

    entity_buffer_mask[entity_index] = false;
    active_entities -= 1;
}

Entity* find_entity(int id) {
    Entity* entity = NULL;
    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i])    continue;
        if (entity_buffer[i].id != id) continue;
        
        entity = &entity_buffer[i];
        break;
    }

    assert(entity);
    return entity;
}

void init_game() {
    world_projection = make_orthographic_matrix(WORLD_LEFT, WORLD_RIGHT, WORLD_TOP, WORLD_BOTTOM);
    gui_projection   = make_orthographic_matrix(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);

    the_player = create_entity(Entity_Type::PLAYER);
    the_enemy  = create_entity(Entity_Type::ENEMY);

    for (int i = 0; i < 4; i++) {
        Entity* asteroid = create_entity(Entity_Type::ASTEROID);
        set_asteroid_size(asteroid, Asteroid_Size::LARGE);

        asteroid->position = make_vector2(
            get_random_between(WORLD_LEFT, WORLD_RIGHT), 
            get_random_between(WORLD_BOTTOM, WORLD_TOP));
    }

    playing_music = play_sound(&music_sound, music_volume, true);
}

void build_entity_hierarchy(Entity* entity) {
    Matrix4 local_transform = make_transform_matrix(entity->position, entity->orientation, entity->scale);
    if (entity->parent) {
        entity->transform = entity->parent->transform * local_transform;
    }
    else {
        entity->transform = local_transform;
    }

    Entity* child = entity->child;
    while (child) {
        build_entity_hierarchy(child);
        child = child->sibling;
    }
}

void draw_entity_hierarchy(Entity* entity, Vector2* layout) {
    Matrix4 transform = make_transform_matrix(*layout);
    set_transform(&transform);

    draw_text("%s", to_string(entity->type));
    layout->y -= font_vertical_advance;

    layout->x += 16.0f;

    Entity* child = entity->child;
    while (child) {
        draw_entity_hierarchy(child, layout);
        child = child->sibling;
    }

    layout->x -= 16.0f;
}

void update_game() {
    music_volume = lerp(music_volume, 0.05f * time.delta, 0.15f);
    set_volume(playing_music, music_volume);

    if (input.key_escape.down) {
        is_showing_menu = !is_showing_menu;
    }

    if (is_showing_menu) {
        
    }
    else {
        for (int i = 0; i < count_of(entity_buffer); i++) {
            if (!entity_buffer_mask[i]) continue;

            Entity* entity = &entity_buffer[i];

            if (entity->on_update) {
                entity->on_update(entity);
            }

            if (entity->position.x < WORLD_LEFT)   entity->position.x = WORLD_RIGHT;
            if (entity->position.x > WORLD_RIGHT)  entity->position.x = WORLD_LEFT;
            if (entity->position.y < WORLD_BOTTOM) entity->position.y = WORLD_TOP;
            if (entity->position.y > WORLD_TOP)    entity->position.y = WORLD_BOTTOM;
        }

        for (int i = 0; i < count_of(entity_buffer); i++) {
            if (!entity_buffer_mask[i]) continue;
            
            Entity* us = &entity_buffer[i];
            if (!us->has_collider) continue;

            for (int j = 0; j < count_of(entity_buffer); j++) {
                if (!entity_buffer_mask[j]) continue;
                
                Entity* them = &entity_buffer[j];
                if (!them->has_collider) continue;
                if (us == them) continue;

                float distance = get_length(them->position - us->position);
                if (distance <= us->collider_radius + them->collider_radius) {
                    us->on_collision(us, them);
                }
            }
        }

        build_entity_hierarchy(&root_entity);
    }

    set_projection(&world_projection);

    for (int x = 0; x < 6; x++) {
        for (int y = 0; y < 3; y++) {
            Vector2 position = make_vector2(WORLD_LEFT, WORLD_BOTTOM);
            position += make_vector2(x, y) * 5.0f;

            Matrix4 transform = make_transform_matrix(position);
            set_transform(&transform);

            draw_sprite(&background_sprite, 5.0f, 5.0f, false);
        }
    }

    for (int i = 0; i < count_of(entity_buffer); i++) {
        if (!entity_buffer_mask[i]) continue;

        Entity* entity = &entity_buffer[i];
        if (entity->sprite && entity->is_visible) {
            Matrix4 sprite_transform = entity->transform * make_transform_matrix(entity->sprite_offset);
            set_transform(&sprite_transform);

            draw_sprite(entity->sprite, entity->sprite_size * entity->sprite->aspect, entity->sprite_size);

            if (entity->has_collider) {
                set_transform(&entity->transform);
                draw_circle(entity->collider_radius, 0.0f, 1.0f, 0.0f, 1.0f, false);
            }

            // draw_entity_at(entity, entity->position);

            // float bounds = entity->sprite->width > entity->sprite->height ? entity->sprite->width : entity->sprite->height;

            // if (entity->position.x - WORLD_LEFT <= bounds) {
            //     float x = WORLD_RIGHT + (entity->position.x - WORLD_LEFT);
            //     draw_entity_at(entity, make_vector2(x, entity->position.y));
            // }

            // if (WORLD_RIGHT - entity->position.x <= bounds) {
            //     float x = WORLD_LEFT - (WORLD_RIGHT - entity->position.x);
            //     draw_entity_at(entity, make_vector2(x, entity->position.y));
            // }

            // if (entity->position.y - WORLD_BOTTOM <= bounds) {
            //     float y = WORLD_TOP + (entity->position.y - WORLD_BOTTOM);
            //     draw_entity_at(entity, make_vector2(entity->position.x, y));
            // }

            // if (WORLD_TOP - entity->position.y <= bounds) {
            //     float y = WORLD_BOTTOM - (WORLD_TOP - entity->position.y);
            //     draw_entity_at(entity, make_vector2(entity->position.x, y));
            // }
        }
    }

    set_projection(&gui_projection);
    
    if (is_showing_menu) {
        Matrix4 transform = make_identity_matrix();
        set_transform(&transform);

        draw_rectangle(WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 0.0f, 0.25f, false);
    }

    Vector2 layout = make_vector2(16.0f, WINDOW_HEIGHT - font_height - 16.0f);

    Matrix4 transform = make_transform_matrix(layout);
    set_transform(&transform);
    
    draw_text("%.2f, %.2f, %i", time.now, time.delta * 1000.0f, (int) (1.0f / time.delta));
    layout.y -= font_vertical_advance;

    transform = make_transform_matrix(layout);
    set_transform(&transform);

    draw_text("Player Score: %i", the_player->player->score);
    layout.y -= font_vertical_advance;

    transform = make_transform_matrix(layout);
    set_transform(&transform);

    draw_text("Player Lives: %i", the_player->player->lives);
    // layout.y -= font_vertical_advance;

    // transform = make_transform_matrix(layout);
    // set_transform(&transform);

    // draw_text("Active Entities: %i", active_entities);
    // layout.y -= font_vertical_advance;
    
    // transform = make_transform_matrix(layout);
    // set_transform(&transform);

    // draw_text("Entity Hierarchy:");
    // layout.y -= font_vertical_advance;

    // layout.x += 16.0f;

    // draw_entity_hierarchy(&root_entity, &layout);

    // layout.x -= 16.0f;
}