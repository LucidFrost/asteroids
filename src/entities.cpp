//
// To add a new entity type, you must do the following:
//   - Add the entity type to the Entity_Type enum
//   - Add the case to the to_string for Entity_Type
//   - Add a pointer to the union on the Entity struct
//   - Include the entity file in the AS_HEADER block
//   - Add a bucket and bucket size for the entity
//   - Add the case in the creation code to call on_create
//   - Include the entity file again but not as a header
//   - Add the for_each to call on_update
//   - Add the case in the collision code to call on_collision
//   - Add the case in the destruction code to call on_destroy
//

// @todo: Use metaprogramming to automate this process

enum Entity_Type {
    ENTITY_TYPE_NONE,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_LASER,
    ENTITY_TYPE_ASTEROID,
    ENTITY_TYPE_ENEMY,
    ENTITY_TYPE_POWERUP
};

utf8* to_string(Entity_Type entity_type) {
    switch (entity_type) {
        case ENTITY_TYPE_NONE:     return "None";
        case ENTITY_TYPE_PLAYER:   return "Player";
        case ENTITY_TYPE_LASER:    return "Laser";
        case ENTITY_TYPE_ASTEROID: return "Asteroid";
        case ENTITY_TYPE_ENEMY:    return "Enemy";
        case ENTITY_TYPE_POWERUP:  return "Powerup";
    }

    return "Invalid";
}

struct Entity {
    u32 id = 0;
    Entity_Type type = ENTITY_TYPE_NONE;

    bool was_just_destroyed = false;
    bool was_just_created   = true;
    
    Entity* parent  = null;
    Entity* child   = null;
    Entity* sibling = null;

    Matrix4 transform;

    Vector2 position;
    f32 orientation = 0.0f;
    f32 scale       = 1.0f;

    Sprite* sprite        = null;
    f32     sprite_size   = 1.0f;
    i32     sprite_order  = 0;
    Vector2 sprite_offset;
    bool    is_visible    = false;

    f32  collider_radius = 0.0f;
    bool has_collider    = false;

    union {
        void* derived = null;

        struct Player*   player;
        struct Laser*    laser;
        struct Asteroid* asteroid;
        struct Enemy*    enemy;
        struct Powerup*  powerup;
    };
};

#define AS_HEADER 1
    #include "entities/asteroid.cpp"
    #include "entities/player.cpp"
    #include "entities/enemy.cpp"
    #include "entities/laser.cpp"
    #include "entities/powerup.cpp"
#undef AS_HEADER

const u32 ENTITIES_BUCKET_SIZE  = 32;
const u32 PLAYERS_BUCKET_SIZE   = 1;
const u32 LASERS_BUCKET_SIZE    = 16;
const u32 ASTEROIDS_BUCKET_SIZE = 16;
const u32 ENEMIES_BUCKET_SIZE   = 1;
const u32 POWERUPS_BUCKET_SIZE  = 4;

Bucket_Array<Entity,   ENTITIES_BUCKET_SIZE>  entities;
Bucket_Array<Player,   PLAYERS_BUCKET_SIZE>   players;
Bucket_Array<Laser,    LASERS_BUCKET_SIZE>    lasers;
Bucket_Array<Asteroid, ASTEROIDS_BUCKET_SIZE> asteroids;
Bucket_Array<Enemy,    ENEMIES_BUCKET_SIZE>   enemies;
Bucket_Array<Powerup,  POWERUPS_BUCKET_SIZE>  powerups;

Entity root_entity;
u32 next_entity_id;
bool should_simulate = true;

Entity* create_entity(Entity_Type type, Entity* parent = &root_entity) {
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
        case ENTITY_TYPE_NONE: {
            break;
        }
        case ENTITY_TYPE_PLAYER: {
            entity->player = next(&players);
            entity->player->entity = entity;

            on_create(entity->player);
            break;
        }
        case ENTITY_TYPE_LASER: {
            entity->laser = next(&lasers);
            entity->laser->entity = entity;

            on_create(entity->laser);
            break;
        }
        case ENTITY_TYPE_ASTEROID: {
            entity->asteroid = next(&asteroids);
            entity->asteroid->entity = entity;

            on_create(entity->asteroid);
            break;
        }
        case ENTITY_TYPE_ENEMY: {
            entity->enemy = next(&enemies);
            entity->enemy->entity = entity;

            on_create(entity->enemy);
            break;
        }
        case ENTITY_TYPE_POWERUP: {
            entity->powerup = next(&powerups);
            entity->powerup->entity = entity;

            on_create(entity->powerup);
            break;
        }
        invalid_default_case();
    }

    next_entity_id += 1;

    return entity;
}

void destroy_entity(Entity* entity) {
    entity->was_just_destroyed = true;

    Entity* child = entity->child;
    while (child) {
        destroy_entity(child);
        child = child->sibling;
    }
}

void clear_entities() {
    for_each (Entity* entity, &entities) {
        destroy_entity(entity);
    }
}

Entity* find_entity(u32 id) {
    Entity* entity = null;

    for_each (Entity* it, &entities) {
        if (it->id != id) continue;
        
        entity = it;
        break;
    }

    return entity;
}

Vector2 get_world_position(Entity* entity) {
    return make_vector2(entity->transform._41, entity->transform._42);
}

void set_sprite(Entity* entity, Sprite* sprite, f32 size = 1.0f, i32 order = 0, Vector2 offset = make_vector2(0.0f, 0.0f)) {
    entity->sprite        = sprite;
    entity->sprite_size   = size;
    entity->sprite_order  = order;
    entity->sprite_offset = offset;
    entity->is_visible    = true;
}

void set_collider(Entity* entity, f32 radius) {
    entity->collider_radius = radius;
    entity->has_collider    = true;
}

#include "entities/asteroid.cpp"
#include "entities/laser.cpp"
#include "entities/player.cpp"
#include "entities/enemy.cpp"
#include "entities/powerup.cpp"

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

void draw_entity_hierarchy(Entity* entity) {
    begin_layout(GUI_ADVANCE_VERTICAL, get_font_line_gap(&font_arial, 18.0f), GUI_ANCHOR_NONE, 16.0f); {
        gui_text(&font_arial, to_string(entity->type), 18.0f);

        Entity* child = entity->child;
        while (child) {
            draw_entity_hierarchy(child);
            child = child->sibling;
        }
    }
    end_layout();
}

Array<Entity*> sort_visible_entities(Array<Entity*> visible_entities) {
    visible_entities.allocator = &temp_allocator;

    if (visible_entities.count < 2) return visible_entities;
    u32 middle = visible_entities.count / 2;
    
    Array<Entity*> left  = copy(&visible_entities, 0, middle);
    Array<Entity*> right = copy(&visible_entities, middle, visible_entities.count);

    left  = sort_visible_entities(left);
    right = sort_visible_entities(right);

    Array<Entity*> sorted;
    sorted.allocator = &temp_allocator;

    while (left.count && right.count) {
        if (left[0]->sprite_order <= right[0]->sprite_order) {
            add(&sorted, left[0]);
            remove(&left, 0);
        }
        else {
            add(&sorted, right[0]);
            remove(&right, 0);
        }
    }

    while (left.count) {
        add(&sorted, left[0]);
        remove(&left, 0);
    }

    while (right.count) {
        add(&sorted, right[0]);
        remove(&right, 0);
    }

    return sorted;
}

void update_entities() {
    for_each (Player* player, &players)       on_update(player);
    for_each (Laser* laser, &lasers)          on_update(laser);
    for_each (Asteroid* asteroid, &asteroids) on_update(asteroid);
    for_each (Enemy* enemy, &enemies)         on_update(enemy);
    for_each (Powerup* powerup, &powerups)    on_update(powerup);

    for_each (Entity* entity, &entities) {
        if (entity->position.x < world_left)   entity->position.x = world_right;
        if (entity->position.x > world_right)  entity->position.x = world_left;
        if (entity->position.y < world_bottom) entity->position.y = world_top;
        if (entity->position.y > world_top)    entity->position.y = world_bottom;
    }

    build_entity_hierarchy(&root_entity);

    for_each (Entity* us, &entities) {
        if (us->was_just_created)   continue;
        if (us->was_just_destroyed) continue;
        if (!us->has_collider)      continue;

        for_each (Entity* them, &entities) {
            if (us == them) continue;

            if (them->was_just_created)   continue;
            if (them->was_just_destroyed) continue;
            if (!them->has_collider)      continue;

            Vector2 us_position   = get_world_position(us);
            Vector2 them_position = get_world_position(them);

            bool did_collide = false;

            Circle circle_us   = make_circle(us_position,   us->collider_radius);
            Circle circle_them = make_circle(them_position, them->collider_radius);

            if (intersects(circle_us, circle_them)) {
                did_collide = true;
            }
            
            // if (position.x - world_left <= bounds) {
            //     f32 mirrored_x = world_right + (position.x - world_left);

            //     Matrix4 new_transform = transform;
            //     new_transform._41 = mirrored_x;

            //     set_transform(new_transform);
            //     draw_sprite(entity->sprite, entity->sprite_size);
            // }

            // if (world_right - position.x <= bounds) {
            //     f32 mirrored_x = world_left - (world_right - position.x);

            //     Matrix4 new_transform = transform;
            //     new_transform._41 = mirrored_x;

            //     set_transform(new_transform);
            //     draw_sprite(entity->sprite, entity->sprite_size);
            // }

            // if (position.y - world_bottom <= bounds) {
            //     f32 mirrored_y = world_top + (position.y - world_bottom);

            //     Matrix4 new_transform = transform;
            //     new_transform._42 = mirrored_y;

            //     set_transform(new_transform);
            //     draw_sprite(entity->sprite, entity->sprite_size);
            // }

            // if (world_top - position.y <= bounds) {
            //     f32 mirrored_y = world_bottom - (world_top - position.y);

            //     Matrix4 new_transform = transform;
            //     new_transform._42 = mirrored_y;

            //     set_transform(new_transform);
            //     draw_sprite(entity->sprite, entity->sprite_size);
            // }

            if (did_collide) {
                switch (us->type) {
                    case ENTITY_TYPE_NONE: {
                        break;
                    }
                    case ENTITY_TYPE_PLAYER: {
                        on_collision(us->player, them);
                        break;
                    }
                    case ENTITY_TYPE_LASER: {
                        on_collision(us->laser, them);
                        break;
                    }
                    case ENTITY_TYPE_ASTEROID: {
                        on_collision(us->asteroid, them);
                        break;
                    }
                    case ENTITY_TYPE_ENEMY: {
                        on_collision(us->enemy, them);
                        break;
                    }
                    case ENTITY_TYPE_POWERUP: {
                        on_collision(us->powerup, them);
                        break;
                    }
                    invalid_default_case();
                }
            }
        }
    }

    for_each (Entity* entity, &entities) {
        if (!entity->was_just_destroyed) continue;

        switch (entity->type) {
            case ENTITY_TYPE_NONE: {
                break;
            }
            case ENTITY_TYPE_PLAYER: {
                on_destroy(entity->player);
                remove(&players, entity->player);

                break;
            }
            case ENTITY_TYPE_LASER: {
                on_destroy(entity->laser);
                remove(&lasers, entity->laser);

                break;
            }
            case ENTITY_TYPE_ASTEROID: {
                on_destroy(entity->asteroid);
                remove(&asteroids, entity->asteroid);

                break;
            }
            case ENTITY_TYPE_ENEMY: {
                on_destroy(entity->enemy);
                remove(&enemies, entity->enemy);

                break;
            }
            case ENTITY_TYPE_POWERUP: {
                on_destroy(entity->powerup);
                remove(&powerups, entity->powerup);

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

    for_each (Entity* entity, &entities) {
        if (!entity->was_just_created) continue;
        entity->was_just_created = false;
    }

    build_entity_hierarchy(&root_entity);
}

void draw_entities() {
    Array<Entity*> visible_entities;
    visible_entities.allocator = &temp_allocator;

    for_each (Entity* entity, &entities) {
        if (!entity->sprite)     continue;
        if (!entity->is_visible) continue;

        add(&visible_entities, entity);
    }
    
    Array<Entity*> sorted_entities = sort_visible_entities(visible_entities);

    for_each (Entity** it, &sorted_entities) {
        Entity* entity = *it;

        Matrix4 transform = entity->transform * make_transform_matrix(entity->sprite_offset);
        Vector2 position  = make_vector2(transform._41, transform._42);

        f32 width  = entity->sprite->aspect * entity->sprite_size;
        f32 height = entity->sprite_size;

        f32 bounds = width > height ? width : height;

        if (position.x - world_left <= bounds) {
            f32 mirrored_x = world_right + (position.x - world_left);

            Matrix4 new_transform = transform;
            new_transform._41 = mirrored_x;

            set_transform(new_transform);
            draw_sprite(entity->sprite, entity->sprite_size);

            #if DEBUG
                if (entity->has_collider) {
                    new_transform = entity->transform;
                    new_transform._41 = mirrored_x;

                    set_transform(new_transform);
                    
                    draw_circle(
                        make_circle(make_vector2(0.0f, 0.0f), entity->collider_radius), 
                        make_color(0.0f, 1.0f, 0.0f), 
                        false);
                }
            #endif
        }

        if (world_right - position.x <= bounds) {
            f32 mirrored_x = world_left - (world_right - position.x);

            Matrix4 new_transform = transform;
            new_transform._41 = mirrored_x;

            set_transform(new_transform);
            draw_sprite(entity->sprite, entity->sprite_size);

            #if DEBUG
                if (entity->has_collider) {
                    new_transform = entity->transform;
                    new_transform._41 = mirrored_x;

                    set_transform(new_transform);
                    
                    draw_circle(
                        make_circle(make_vector2(0.0f, 0.0f), entity->collider_radius), 
                        make_color(0.0f, 1.0f, 0.0f), 
                        false);
                }
            #endif
        }

        if (position.y - world_bottom <= bounds) {
            f32 mirrored_y = world_top + (position.y - world_bottom);

            Matrix4 new_transform = transform;
            new_transform._42 = mirrored_y;

            set_transform(new_transform);
            draw_sprite(entity->sprite, entity->sprite_size);

            #if DEBUG
                if (entity->has_collider) {
                    new_transform = entity->transform;
                    new_transform._42 = mirrored_y;

                    set_transform(new_transform);
                    
                    draw_circle(
                        make_circle(make_vector2(0.0f, 0.0f), entity->collider_radius), 
                        make_color(0.0f, 1.0f, 0.0f), 
                        false);
                }
            #endif
        }

        if (world_top - position.y <= bounds) {
            f32 mirrored_y = world_bottom - (world_top - position.y);

            Matrix4 new_transform = transform;
            new_transform._42 = mirrored_y;

            set_transform(new_transform);
            draw_sprite(entity->sprite, entity->sprite_size);

            #if DEBUG
                if (entity->has_collider) {
                    new_transform = entity->transform;
                    new_transform._42 = mirrored_y;

                    set_transform(new_transform);

                    draw_circle(
                        make_circle(make_vector2(0.0f, 0.0f), entity->collider_radius), 
                        make_color(0.0f, 1.0f, 0.0f), 
                        false);
                }
            #endif
        }

        set_transform(transform);
        draw_sprite(entity->sprite, entity->sprite_size);

        #if DEBUG
            if (entity->has_collider) {
                set_transform(entity->transform);
                
                draw_circle(
                    make_circle(make_vector2(0.0f, 0.0f), entity->collider_radius), 
                    make_color(0.0f, 1.0f, 0.0f), 
                    false);
            }
        #endif
    }
}