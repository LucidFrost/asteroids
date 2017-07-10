#include "platform.cpp"
#include "math.cpp"
#include "data_structures.cpp"

Allocator heap_allocator;
Allocator temp_allocator;

#include "draw.cpp"
#include "sound.cpp"
#include "assets.cpp"
#include "gui.cpp"

f32 world_height;
f32 world_width;

f32 world_left;
f32 world_right;
f32 world_top;
f32 world_bottom;

Matrix4 world_projection;

Playing_Sound* playing_music;
f32 music_volume;

enum Entity_Type {
    ENTITY_TYPE_NONE,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_LASER,
    ENTITY_TYPE_ASTEROID,
    ENTITY_TYPE_ENEMY
};

utf8* to_string(Entity_Type entity_type) {
    switch (entity_type) {
        case ENTITY_TYPE_NONE: return "None";
        case ENTITY_TYPE_PLAYER: return "Player";
        case ENTITY_TYPE_LASER: return "Laser";
        case ENTITY_TYPE_ASTEROID: return "Asteroid";
        case ENTITY_TYPE_ENEMY: return "Enemy";
    }

    return "Invalid";
}

struct Player;
struct Laser;
struct Asteroid;
struct Enemy;

struct Entity {
    u32 id = 0;
    Entity_Type type = ENTITY_TYPE_NONE;

    bool needs_to_be_destroyed = false;
    
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
        void*     derived = null;
        Player*   player;
        Laser*    laser;
        Asteroid* asteroid;
        Enemy*    enemy;
    };
};

#define AS_HEADER 1
    #include "entities/asteroid.cpp"
    #include "entities/player.cpp"
    #include "entities/enemy.cpp"
    #include "entities/laser.cpp"
#undef AS_HEADER

const u32 ENTITIES_BUCKET_SIZE  = 32;
const u32 PLAYERS_BUCKET_SIZE   = 1;
const u32 LASERS_BUCKET_SIZE    = 16;
const u32 ASTEROIDS_BUCKET_SIZE = 16;
const u32 ENEMIES_BUCKET_SIZE   = 1;

Bucket_Array<Entity,   ENTITIES_BUCKET_SIZE>  entities;
Bucket_Array<Player,   PLAYERS_BUCKET_SIZE>   players;
Bucket_Array<Laser,    LASERS_BUCKET_SIZE>    lasers;
Bucket_Array<Asteroid, ASTEROIDS_BUCKET_SIZE> asteroids;
Bucket_Array<Enemy,    ENEMIES_BUCKET_SIZE>   enemies;

Entity root_entity;
u32 next_entity_id;
bool simulate_entities = true;

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
        invalid_default_case();
    }

    next_entity_id += 1;

    return entity;
}

void destroy_entity(Entity* entity) {
    entity->needs_to_be_destroyed = true;
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

Vector2 get_world_position(i32 screen_x, i32 screen_y) {
    return unproject(screen_x, screen_y, platform.window_width, platform.window_height, world_projection);
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

enum Game_Mode {
    GAME_MODE_NONE,
    GAME_MODE_MENU,
    GAME_MODE_PLAY
};

utf8* to_string(Game_Mode game_mode) {
    #define case(type) case type: return #type

    switch (game_mode) {
        case(GAME_MODE_NONE);
        case(GAME_MODE_MENU);
        case(GAME_MODE_PLAY);
    }

    #undef case

    return "INVALID";
}

Game_Mode game_mode;
void switch_game_mode(Game_Mode new_game_mode);

#include "game_modes/menu.cpp"
#include "game_modes/play.cpp"

void switch_game_mode(Game_Mode new_game_mode) {
    switch (game_mode) {
        case GAME_MODE_NONE: {
            break;
        }
        case GAME_MODE_MENU: {
            stop_menu();
            break;
        }
        case GAME_MODE_PLAY: {
            stop_play();
            break;
        }
        invalid_default_case();
    }

    switch (new_game_mode) {
        case GAME_MODE_MENU: {
            start_menu();
            break;
        }
        case GAME_MODE_PLAY: {
            start_play();
            break;
        }
        invalid_default_case();
    }

    printf("Game mode switched from '%s' to '%s'\n", to_string(game_mode), to_string(new_game_mode));
    game_mode = new_game_mode;
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

template<typename type, u32 size>
void draw_bucket_storage(Bucket_Array<type, size>* bucket_array) {
    begin_layout(GUI_ADVANCE_VERTICAL); {
        for (u32 i = 0; i < bucket_array->buckets.count; i++) {
            gui_pad(5.0f);

            begin_layout(GUI_ADVANCE_HORIZONTAL); {
                for (u32 j = 0; j < size; j++) {
                    if (bucket_array->buckets[i]->occupied[j]) {
                        gui_rectangle(16.0f, 16.0f, make_color(0.0f, 0.0f, 0.0f), make_color(1.0f, 0.0f, 0.0f));
                    }
                    else {
                        gui_rectangle(16.0f, 16.0f, make_color(0.0f, 0.0f, 0.0f));
                    }
                }
            }
            end_layout();

            gui_pad(5.0f);
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

void update_world_projection() {
    world_height = 15.0f;
    world_width  = world_height * ((f32) platform.window_width / (f32) platform.window_height);

    world_left   = -world_width  / 2.0f;
    world_right  =  world_width  / 2.0f;
    world_top    =  world_height / 2.0f;
    world_bottom = -world_height / 2.0f;

    world_projection = make_orthographic_matrix(world_left, world_right, world_top, world_bottom, -10.0f, 10.0f);
}

i32 main() {
    seed_random();
    init_platform();

    heap_allocator    = make_allocator(heap_alloc, heap_dealloc);
    temp_allocator    = make_allocator(temp_alloc, temp_dealloc);
    default_allocator = heap_allocator;
    
    init_draw();
    init_sound();
    
    load_settings();
    show_window();
    
    load_assets();

    playing_music = play_sound(&sound_music, music_volume, true);
    update_world_projection();

    switch_game_mode(GAME_MODE_MENU);

    while (!platform.should_quit) {
        update_platform();
        update_sound();

        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, platform.window_width, platform.window_height);

        update_world_projection();
        gui_begin();

        music_volume = lerp(music_volume, 0.05f * timers.delta, 0.5f);
        set_volume(playing_music, music_volume);

        if (simulate_entities) {
            for_each (Player* player, &players)       on_update(player);
            for_each (Laser* laser, &lasers)          on_update(laser);
            for_each (Asteroid* asteroid, &asteroids) on_update(asteroid);
            for_each (Enemy* enemy, &enemies)         on_update(enemy);

            for_each (Entity* entity, &entities) {
                if (entity->position.x < world_left)   entity->position.x = world_right;
                if (entity->position.x > world_right)  entity->position.x = world_left;
                if (entity->position.y < world_bottom) entity->position.y = world_top;
                if (entity->position.y > world_top)    entity->position.y = world_bottom;
            }

            build_entity_hierarchy(&root_entity);

            for_each (Entity* us, &entities) {
                if (!us->has_collider) continue;

                for_each (Entity* them, &entities) {
                    if (us == them)          continue;
                    if (!them->has_collider) continue;

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
                            invalid_default_case();
                        }
                    }
                }
            }
        }

        for_each (Entity* entity, &entities) {
            if (!entity->needs_to_be_destroyed) continue;

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
        set_projection(world_projection);

        f32 tile_size = 5.0f;
        
        u32 tiles_x = (u32) (world_width  / tile_size) + 1;
        u32 tiles_y = (u32) (world_height / tile_size) + 1;
        
        for (u32 x = 0; x < tiles_x; x++) {
            for (u32 y = 0; y < tiles_y; y++) {
                Vector2 position = make_vector2(world_left, world_bottom);
                position += make_vector2((f32) x, (f32) y) * tile_size;

                set_transform(make_transform_matrix(position));
                draw_sprite(&sprite_background, tile_size, false);
            }
        }

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

        switch (game_mode) {
            case GAME_MODE_MENU: {
                update_menu();
                break;
            }
            case GAME_MODE_PLAY: {
                update_play();
                break;
            }
            invalid_default_case();
        }

        #if DEBUG
            begin_layout(GUI_ADVANCE_VERTICAL, get_font_line_gap(&font_arial, 18.0f), GUI_ANCHOR_TOP_LEFT, 10.0f, 10.0f); {
                gui_text(&font_arial, "Timers:", 18.0f);

                begin_layout(GUI_ADVANCE_VERTICAL, get_font_line_gap(&font_arial, 18.0f), GUI_ANCHOR_NONE, 16.0f); {
                    gui_text(&font_arial, format_string("Now: %.2f", timers.now), 18.0f);
                    gui_text(&font_arial, format_string("Frame: %fms", timers.delta * 1000.0f), 18.0f);
                    gui_text(&font_arial, format_string("Fps: %u", (u32) (1.0f / timers.delta)), 18.0f);
                }
                end_layout();

                gui_text(&font_arial, "Storage:", 18.0f);

                begin_layout(GUI_ADVANCE_VERTICAL, GUI_ANCHOR_NONE, 16.0f); {
                    gui_text(&font_arial, format_string("Total: %.2f mb", platform.heap_memory_allocated / 1024.0f), 18.0f);
                    gui_pad(get_font_line_gap(&font_arial, 18.0f));

                    gui_text(
                        &font_arial, 
                        format_string("Temp: %.2f kb / %.2f kb (max: %.2f kb)", 
                            platform.temp_memory_allocated / 1024.0f, 
                            TEMP_MEMORY_SIZE / 1024.0f, 
                            platform.temp_memory_high_water_mark / 1024.0f),
                        18.0f);

                    gui_pad(5.0f);

                    begin_layout(GUI_ADVANCE_HORIZONTAL); {
                        f32 full = (f32) platform.temp_memory_allocated / (f32) TEMP_MEMORY_SIZE;
                        f32 high = (f32) platform.temp_memory_high_water_mark / (f32) TEMP_MEMORY_SIZE;

                        f32 empty = 1.0f - high;

                        gui_rectangle(256.0f * full,          16.0f, make_color(0.0f, 0.0f, 0.0f), make_color(1.0f, 1.0f, 0.0f));
                        gui_rectangle(256.0f * (high - full), 16.0f, make_color(0.0f, 0.0f, 0.0f), make_color(0.0f, 1.0f, 1.0f));
                        gui_rectangle(256.0f * empty,         16.0f, make_color(0.0f, 0.0f, 0.0f));
                    }
                    end_layout();
                    
                    gui_pad(5.0f);

                    gui_text(
                        &font_arial, 
                        format_string("Entities: %u / %u", entities.count, entities.buckets.count * ENTITIES_BUCKET_SIZE), 
                        18.0f);

                    draw_bucket_storage(&entities);

                    gui_text(
                        &font_arial, 
                        format_string("Players: %u / %u", players.count, players.buckets.count * PLAYERS_BUCKET_SIZE), 
                        18.0f);
                        
                    draw_bucket_storage(&players);

                    gui_text(
                        &font_arial, 
                        format_string("Lasers: %u / %u", lasers.count, lasers.buckets.count * LASERS_BUCKET_SIZE), 
                        18.0f);
                        
                    draw_bucket_storage(&lasers);

                    gui_text(
                        &font_arial, 
                        format_string("Asteroids: %u / %u", asteroids.count, asteroids.buckets.count * ASTEROIDS_BUCKET_SIZE), 
                        18.0f);
                        
                    draw_bucket_storage(&asteroids);
                    
                    gui_text(
                        &font_arial, 
                        format_string("Enemies: %u / %u", enemies.count, enemies.buckets.count * ENEMIES_BUCKET_SIZE), 
                        18.0f);
                        
                    draw_bucket_storage(&enemies);
                }
                end_layout();

                gui_text(&font_arial, "Hierarchy:", 18.0f);
                draw_entity_hierarchy(&root_entity);
            }
            end_layout();
        #endif
        
        gui_end();
        swap_buffers();
    }
    
    return 0;
}