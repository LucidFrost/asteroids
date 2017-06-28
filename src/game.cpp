const f32 WORLD_HEIGHT = 15.0f;
const f32 WORLD_WIDTH  = WORLD_HEIGHT * ((f32) WINDOW_WIDTH / (f32) WINDOW_HEIGHT);

const f32 WORLD_LEFT   = -WORLD_WIDTH  / 2.0f;
const f32 WORLD_RIGHT  =  WORLD_WIDTH  / 2.0f;
const f32 WORLD_TOP    =  WORLD_HEIGHT / 2.0f;
const f32 WORLD_BOTTOM = -WORLD_HEIGHT / 2.0f;

Matrix4 world_projection;
Matrix4 gui_projection;

Playing_Sound* playing_music;
f32 music_volume;

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

utf8* to_string(Entity_Type entity_type) {
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
    u32 id = 0;
    Entity_Type type = Entity_Type::NONE;

    bool needs_to_be_destroyed = false;
    
    Entity* parent  = null;
    Entity* child   = null;
    Entity* sibling = null;

    Matrix4 transform;

    Vector2 position;
    f32 orientation = 0.0f;
    f32 scale       = 1.0f;

    bool has_collider = false;
    f32 collider_radius = 0.0f;

    Sprite* sprite = null;
    Vector2 sprite_offset;

    f32 sprite_size  = 1.0f;
    i32 sprite_order = 0;
    bool is_visible  = false;

    Entity_On_Create*    on_create    = null;
    Entity_On_Destroy*   on_destroy   = null;
    Entity_On_Update*    on_update    = null;
    Entity_On_Collision* on_collision = null;

    union {
        void*     derived = null;
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
Entity* find_entity(u32 id);

void show_entity(Entity* entity);
void hide_entity(Entity* entity);

void set_collider(Entity* entity, f32 radius);

#include "entity_asteroid.cpp"
#include "entity_player.cpp"
#include "entity_enemy.cpp"
#include "entity_laser.cpp"

Bucket_Array<Entity,   32> entities;
Bucket_Array<Player,   1>  players;
Bucket_Array<Laser,    16> lasers;
Bucket_Array<Asteroid, 16> asteroids;
Bucket_Array<Enemy,    1>  enemies;

u32 next_entity_id;

Entity* create_entity(Entity_Type type, Entity* parent) {
    Entity* entity = next(&entities);

    entity->id     = next_entity_id;
    entity->type   = type;
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

    #define case(TYPE, Type, type, types)                   \
        case Entity_Type::TYPE: {                           \
            entity->on_create    = type##_on_create;        \
            entity->on_destroy   = type##_on_destroy;       \
            entity->on_update    = type##_on_update;        \
            entity->on_collision = type##_on_collision;     \
                                                            \
            entity->type = next(&types);                    \
            break;                                          \
        }

    switch (entity->type) {
        case Entity_Type::NONE: {
            break;
        }

        case(PLAYER,   Player,   player,   players);
        case(LASER,    Laser,    laser,    lasers);
        case(ASTEROID, Asteroid, asteroid, asteroids);
        case(ENEMY,    Enemy,    enemy,    enemies);

        default: {
            printf("Failed to create entity, unhandled entity type '%s' (%i) specified\n", to_string(entity->type), entity->type);
            assert(false);

            break;
        }
    }

    #undef case

    next_entity_id += 1;

    if (entity->on_create) {
        entity->on_create(entity);
    }

    return entity;
}

void destroy_entity(Entity* entity) {
    entity->needs_to_be_destroyed = true;
}

void actually_destroy_entity(Entity* entity) {
    if (entity->on_destroy) {
        entity->on_destroy(entity);
    }

    #define case(TYPE, type, types)         \
        case Entity_Type::TYPE: {           \
            remove(&types, entity->type);   \
            break;                          \
        }

    switch (entity->type) {
        case Entity_Type::NONE: {
            break;
        }

        case(PLAYER,   player,   players);
        case(LASER,    laser,    lasers);
        case(ASTEROID, asteroid, asteroids);
        case(ENEMY,    enemy,    enemies);

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

    remove(&entities, entity);
}

Entity* find_entity(u32 id) {
    Entity* entity = null;

    for_each (Entity* it, &entities) {
        if (it->id != id) continue;
        
        entity = it;
        break;
    }

    assert(entity);
    return entity;
}

void show_entity(Entity* entity) {
    assert(entity->sprite);
    assert(!entity->is_visible);

    entity->is_visible = true;
}

void hide_entity(Entity* entity) {
    assert(entity->is_visible);
    entity->is_visible = false;
}

void set_collider(Entity* entity, f32 radius) {
    entity->collider_radius = radius;
    entity->has_collider    = true;
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

Array<Entity*> merge_sort(Array<Entity*> entities) {
    entities.allocator = &temp_allocator;

    if (entities.count < 2) return entities;
    u32 middle = entities.count / 2;
    
    Array<Entity*> left  = copy(&entities, 0, middle);
    Array<Entity*> right = copy(&entities, middle, entities.count);

    left  = merge_sort(left);
    right = merge_sort(right);

    Array<Entity*> merged;
    merged.allocator = &temp_allocator;

    while (left.count && right.count) {
        if (left[0]->sprite_order <= right[0]->sprite_order) {
            add(&merged, left[0]);
            remove(&left, 0);
        }
        else {
            add(&merged, right[0]);
            remove(&right, 0);
        }
    }

    while (left.count) {
        add(&merged, left[0]);
        remove(&left, 0);
    }

    while (right.count) {
        add(&merged, right[0]);
        remove(&right, 0);
    }

    return merged;
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

void init_game() {
    world_projection = make_orthographic_matrix(WORLD_LEFT, WORLD_RIGHT, WORLD_TOP, WORLD_BOTTOM, -10.0f, 10.0f);
    gui_projection   = make_orthographic_matrix(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);

    the_player = create_entity(Entity_Type::PLAYER);
    the_enemy  = create_entity(Entity_Type::ENEMY);

    for (u32 i = 0; i < 4; i++) {
        Entity* asteroid = create_entity(Entity_Type::ASTEROID);
        set_asteroid_size(asteroid, Asteroid_Size::LARGE);

        asteroid->position = make_vector2(
            get_random_between(WORLD_LEFT, WORLD_RIGHT), 
            get_random_between(WORLD_BOTTOM, WORLD_TOP));
    }

    playing_music = play_sound(&music_sound, music_volume, true);
}

void update_game() {
    music_volume = lerp(music_volume, 0.05f * time.delta, 0.15f);
    set_volume(playing_music, music_volume);

    if (input.key_escape.down || input.gamepad_start.down) {
        is_showing_menu = !is_showing_menu;
    }

    if (is_showing_menu) {
        
    }
    else {
        for_each (Entity* entity, &entities) {
            if (entity->on_update) {
                entity->on_update(entity);
            }

            if (entity->position.x < WORLD_LEFT)   entity->position.x = WORLD_RIGHT;
            if (entity->position.x > WORLD_RIGHT)  entity->position.x = WORLD_LEFT;
            if (entity->position.y < WORLD_BOTTOM) entity->position.y = WORLD_TOP;
            if (entity->position.y > WORLD_TOP)    entity->position.y = WORLD_BOTTOM;
        }

        for_each (Entity* us, &entities) {
            if (!us->has_collider) continue;

            for_each (Entity* them, &entities) {
                if (us == them)          continue;
                if (!them->has_collider) continue;

                f32 distance = get_length(them->position - us->position);
                if (distance <= us->collider_radius + them->collider_radius) {
                    us->on_collision(us, them);
                }
            }
        }

        for_each (Entity* entity, &entities) {
            if (!entity->needs_to_be_destroyed) continue;
            actually_destroy_entity(entity);
        }

        build_entity_hierarchy(&root_entity);
    }

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    set_projection(&world_projection);

    for (u32 x = 0; x < 6; x++) {
        for (u32 y = 0; y < 3; y++) {
            Vector2 position = make_vector2(WORLD_LEFT, WORLD_BOTTOM);
            position += make_vector2(x, y) * 5.0f;

            Matrix4 transform = make_transform_matrix(position);
            set_transform(&transform);

            draw_sprite(&background_sprite, 5.0f, 5.0f, false);
        }
    }

    Array<Entity*> visible_entities;
    visible_entities.allocator = &temp_allocator;

    for_each (Entity* entity, &entities) {
        if (!entity->sprite)     continue;
        if (!entity->is_visible) continue;

        add(&visible_entities, entity);
    }
    
    Array<Entity*> sorted_entities = merge_sort(visible_entities);

    for_each (Entity** it, &sorted_entities) {
        Entity* entity = *it;

        Matrix4 sprite_transform = entity->transform * make_transform_matrix(entity->sprite_offset);
        set_transform(&sprite_transform);

        draw_sprite(entity->sprite, entity->sprite_size * entity->sprite->aspect, entity->sprite_size);

        // if (entity->has_collider) {
        //     set_transform(&entity->transform);
        //     draw_circle(entity->collider_radius, 0.0f, 1.0f, 0.0f, 1.0f, false);
        // }
    }

    set_projection(&gui_projection);
    
    if (is_showing_menu) {
        Matrix4 transform = make_identity_matrix();
        set_transform(&transform);

        draw_rectangle(WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 0.0f, 0.25f, false);
    }

    begin_layout(16.0f, WINDOW_HEIGHT - font_height - 16.0f); {
        draw_text("%.2f, %.2f, %i", time.now, time.delta * 1000.0f, (u32) (1.0f / time.delta));
        
        draw_text("%u (%u), %u (%u)", 
            heap_memory_allocated, 
            heap_memory_high_water_mark,
            temp_memory_allocated,
            temp_memory_high_water_mark);

        draw_text("Player Score: %i", the_player->player->score);
        draw_text("Player Lives: %i", the_player->player->lives);
    }
    end_layout();
}