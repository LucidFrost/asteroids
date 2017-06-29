// @todo:
//  - Screen shake when lasers hit
//  - Particle effects
//  - A dev console
//

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

struct Player;
struct Laser;
struct Asteroid;
struct Enemy;

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

    union {
        void*     derived = null;
        Player*   player;
        Laser*    laser;
        Asteroid* asteroid;
        Enemy*    enemy;
    };
};

// @note: It was impossible to write gameplay code without moving the struct defines
// out of the .cpp files. It was more complex to try and work around the lack of
// defined types than to just seperate the structs from their files.

struct Player {
    Entity* entity = null;

    Entity* left_thrust  = null;
    Entity* right_thrust = null;

    Vector2 velocity;
    Vector2 desired_direction;

    u32 last_mouse_x = 0;
    u32 last_mouse_y = 0;

    u32 score = 0;
    u32 score_since_last_life = 0;
    u32 lives = 3;

    bool is_dead = true;
};

struct Laser {
    Entity* entity  = null;
    Entity* shooter = null;

    f32 lifetime = 1.0f;
};

enum struct Enemy_Mode {
    NONE,
    BIG,
    SMALL
};

struct Enemy {
    Entity* entity = null;

    Enemy_Mode mode = Enemy_Mode::NONE;
    u32 score = 0;

    f32 respawn_timer = 0.0f;
    bool is_dead = true;

    f32 fire_rate = 0.0f;
    f32 next_fire = 0.0f;

    Vector2 velocity;
};

enum struct Asteroid_Size {
    NONE,
    SMALL,
    MEDIUM,
    LARGE
};

struct Asteroid {
    Entity* entity = null;

    Asteroid_Size size = Asteroid_Size::NONE;
    u32 score = 0;

    Vector2 velocity;
    f32 rotation_speed = 0.0f;
};

Entity root_entity;

Player* the_player;
Enemy*  the_enemy;

Entity* create_entity(Entity_Type type, Entity* parent = &root_entity);
void    destroy_entity(Entity* entity);
Entity* find_entity(u32 id);

#include "entity_asteroid.cpp"
#include "entity_player.cpp"
#include "entity_enemy.cpp"
#include "entity_laser.cpp"

// @todo: Once the game is more settled, measure and test various values here
// to find the most efficient bucket size

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

    switch (entity->type) {
        case Entity_Type::NONE: {
            break;
        }
        case Entity_Type::PLAYER: {
            entity->player = next(&players);
            entity->player->entity = entity;

            on_create(entity->player);
            break;
        }
        case Entity_Type::LASER: {
            entity->laser = next(&lasers);
            entity->laser->entity = entity;

            on_create(entity->laser);
            break;
        }
        case Entity_Type::ASTEROID: {
            entity->asteroid = next(&asteroids);
            entity->asteroid->entity = entity;

            on_create(entity->asteroid);
            break;
        }
        case Entity_Type::ENEMY: {
            entity->enemy = next(&enemies);
            entity->enemy->entity = entity;

            on_create(entity->enemy);
            break;
        }
        invalid_default_case();
    }

    next_entity_id += 1;

    return entity;
}

void destroy_entity(Entity* entity) {
    entity->needs_to_be_destroyed = true;
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

// void draw_entity_hierarchy(Entity* entity, Vector2* layout) {
//     set_transform(*layout);

//     draw_text("%s", to_string(entity->type));
//     layout->y -= font_vertical_advance;

//     layout->x += 16.0f;

//     Entity* child = entity->child;
//     while (child) {
//         draw_entity_hierarchy(child, layout);
//         child = child->sibling;
//     }

//     layout->x -= 16.0f;
// }

void init_game() {
    world_projection = make_orthographic_matrix(WORLD_LEFT, WORLD_RIGHT, WORLD_TOP, WORLD_BOTTOM, -10.0f, 10.0f);
    gui_projection   = make_orthographic_matrix(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);

    the_player = create_entity(Entity_Type::PLAYER)->player;
    the_enemy  = create_entity(Entity_Type::ENEMY)->enemy;

    for (u32 i = 0; i < 4; i++) {
        Asteroid* asteroid = create_entity(Entity_Type::ASTEROID)->asteroid;
        set_asteroid_size(asteroid, Asteroid_Size::LARGE);

        asteroid->entity->position = make_vector2(
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
            switch (entity->type) {
                case Entity_Type::NONE: {
                    break;
                }
                case Entity_Type::PLAYER: {
                    on_update(entity->player);
                    break;
                }
                case Entity_Type::LASER: {
                    on_update(entity->laser);
                    break;
                }
                case Entity_Type::ASTEROID: {
                    on_update(entity->asteroid);
                    break;
                }
                case Entity_Type::ENEMY: {
                    on_update(entity->enemy);
                    break;
                }
                invalid_default_case();
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
                    switch (us->type) {
                        case Entity_Type::NONE: {
                            break;
                        }
                        case Entity_Type::PLAYER: {
                            on_collision(us->player, them);
                            break;
                        }
                        case Entity_Type::LASER: {
                            on_collision(us->laser, them);
                            break;
                        }
                        case Entity_Type::ASTEROID: {
                            on_collision(us->asteroid, them);
                            break;
                        }
                        case Entity_Type::ENEMY: {
                            on_collision(us->enemy, them);
                            break;
                        }
                        invalid_default_case();
                    }
                }
            }
        }

        for_each (Entity* entity, &entities) {
            if (!entity->needs_to_be_destroyed) continue;

            switch (entity->type) {
                case Entity_Type::NONE: {
                    break;
                }
                case Entity_Type::PLAYER: {
                    on_destroy(entity->player);
                    remove(&players, entity->player);

                    break;
                }
                case Entity_Type::LASER: {
                    on_destroy(entity->laser);
                    remove(&lasers, entity->laser);

                    break;
                }
                case Entity_Type::ASTEROID: {
                    on_destroy(entity->asteroid);
                    remove(&asteroids, entity->asteroid);

                    break;
                }
                case Entity_Type::ENEMY: {
                    on_destroy(entity->enemy);
                    remove(&enemies, entity->enemy);

                    break;
                }
                invalid_default_case();
            }

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

        build_entity_hierarchy(&root_entity);
    }

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    set_projection(&world_projection);

    for (u32 x = 0; x < 6; x++) {
        for (u32 y = 0; y < 3; y++) {
            Vector2 position = make_vector2(WORLD_LEFT, WORLD_BOTTOM);
            position += make_vector2(x, y) * 5.0f;

            set_transform(position);
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
    
    for (u32 i = 0; i < the_player->lives; i++) {
        float width  = player_life_sprite.width  * 1.25f;
        float height = player_life_sprite.height * 1.25f;

        set_transform(make_vector2(50.0f + (i * (width + 15.0f)), 50.0f), -45.0f);
        draw_sprite(&player_life_sprite, width, height);
    }

    set_transform(make_vector2(50.0f - (player_life_sprite.width / 2.0f), 50.0f + (player_life_sprite.height * 1.25f) + 10.0f));
    draw_text(&font_future, 45.0f, format_string("%u", the_player->score), 1.0f, 1.0f, 1.0f, 1.0f);

    if (is_showing_menu) {
        set_transform(make_vector2());
        draw_rectangle(WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 0.0f, 0.25f, false);
    }

    begin_layout(16.0f, WINDOW_HEIGHT - get_text_height(&font_arial, 24.0f) - 16.0f); {
        draw_text(
            &font_arial, 
            24.0f,
            format_string("%.2f, %.2f, %i", time.now, time.delta * 1000.0f, (u32) (1.0f / time.delta)), 
            1.0f, 1.0f, 1.0f, 1.0f);
        
        draw_text(
            &font_arial, 
            24.0f,
            format_string("%u, %u", heap_memory_allocated, temp_memory_allocated),
            1.0f, 1.0f, 1.0f, 1.0f);
    }
    end_layout();
}