#include "platform.cpp"
#include "math.cpp"
#include "data_structures.cpp"

Allocator heap_allocator;
Allocator temp_allocator;

#include "draw.cpp"
#include "sound.cpp"

// @todo:
//  - Screen shake when lasers hit
//  - Particle effects
//  - A dev console
//

f32 world_height;
f32 world_width;

f32 world_left;
f32 world_right;
f32 world_top;
f32 world_bottom;

Matrix4 world_projection;
Matrix4 gui_projection;

Playing_Sound* playing_music;
f32 music_volume;

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
    Entity* shield       = null;

    Vector2 velocity;
    Vector2 desired_direction;

    bool has_shield   = false;
    f32  shield_timer = 0.0f;

    i32 last_mouse_x = 0;
    i32 last_mouse_y = 0;

    u32 score = 0;
    u32 score_since_last_life = 0;
};

struct Laser {
    Entity* entity = null;
    u32 shooter_id = 0;

    f32 lifetime = 1.0f;
};

enum struct Enemy_Mode {
    NONE,
    EASY,
    HARD
};

struct Enemy {
    Entity* entity = null;

    Enemy_Mode mode = Enemy_Mode::NONE;
    u32 score = 0;

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

Entity* create_entity(Entity_Type type, Entity* parent = &root_entity);
void    destroy_entity(Entity* entity);
Entity* find_entity(u32 id);

// @todo: Once the game is more settled, measure and test various values here
// to find the most efficient bucket sizes

Bucket_Array<Entity,   32> entities;
Bucket_Array<Player,   1>  players;
Bucket_Array<Laser,    16> lasers;
Bucket_Array<Asteroid, 16> asteroids;
Bucket_Array<Enemy,    1>  enemies;

#include "entities/asteroid.cpp"
#include "entities/player.cpp"
#include "entities/enemy.cpp"
#include "entities/laser.cpp"

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

    assert(entity);
    return entity;
}

bool does_collide(Entity* a, Entity* b) {
    return get_length(b->position - a->position) <= a->collider_radius + b->collider_radius;
}

enum struct Game_Mode {
    NONE,
    MENU,
    PLAY
};

utf8* to_string(Game_Mode game_mode) {
    #define case(type) case Game_Mode::type: return #type

    switch (game_mode) {
        case(NONE);
        case(MENU);
        case(PLAY);
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
        case Game_Mode::NONE: {
            break;
        }
        case Game_Mode::MENU: {
            stop_menu();
            break;
        }
        case Game_Mode::PLAY: {
            stop_play();
            break;
        }
        invalid_default_case();
    }

    switch (new_game_mode) {
        case Game_Mode::MENU: {
            start_menu();
            break;
        }
        case Game_Mode::PLAY: {
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

void update_projections() {
    world_height = 15.0f;
    world_width  = world_height * ((f32) platform.window_width / (f32) platform.window_height);

    world_left   = -world_width  / 2.0f;
    world_right  =  world_width  / 2.0f;
    world_top    =  world_height / 2.0f;
    world_bottom = -world_height / 2.0f;

    world_projection = make_orthographic_matrix(world_left, world_right, world_top, world_bottom, -10.0f, 10.0f);
    gui_projection   = make_orthographic_matrix(0.0f, (f32) platform.window_width, (f32) platform.window_height, 0.0f);
}

i32 main() {
    init_platform();
    seed_random();

    heap_allocator = make_allocator(heap_alloc, heap_dealloc);
    temp_allocator = make_allocator(temp_alloc, temp_dealloc);

    default_allocator = heap_allocator;

    FILE* settings_file = fopen(format_string("%s/settings.txt", get_executable_directory()), "rb");
    if (settings_file) {
        utf8 fullscreen[4];
        fscanf(settings_file, "fullscreen=%s", fullscreen);
        if (compare(fullscreen, "yes")) {
            toggle_fullscreen();
        }

        printf("Read settings from 'settings.txt'\n");
        fclose(settings_file);
    }
    else {
        printf("Failed to read settings from 'settings.txt', the file does not exist\n");
    }

    show_window();

    init_draw();
    init_sound();

    playing_music = play_sound(&music_sound, music_volume, true);
    
    update_projections();
    switch_game_mode(Game_Mode::MENU);

    while (!platform.should_quit) {
        update_platform();
        update_sound();
    
        update_projections();

        music_volume = lerp(music_volume, 0.05f * timers.delta, 0.5f);
        set_volume(playing_music, music_volume);

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

            if (entity->position.x < world_left)   entity->position.x = world_right;
            if (entity->position.x > world_right)  entity->position.x = world_left;
            if (entity->position.y < world_bottom) entity->position.y = world_top;
            if (entity->position.y > world_top)    entity->position.y = world_bottom;
        }

        for_each (Entity* us, &entities) {
            if (!us->has_collider) continue;

            for_each (Entity* them, &entities) {
                if (us == them)          continue;
                if (!them->has_collider) continue;

                if (does_collide(us, them)) {
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

        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, platform.window_width, platform.window_height);
        set_projection(world_projection);

        for (u32 x = 0; x < 6; x++) {
            for (u32 y = 0; y < 3; y++) {
                Vector2 position = make_vector2(world_left, world_bottom);
                position += make_vector2((f32) x, (f32) y) * 5.0f;

                set_transform(make_transform_matrix(position));
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
        
        Array<Entity*> sorted_entities = sort_visible_entities(visible_entities);

        for_each (Entity** it, &sorted_entities) {
            Entity* entity = *it;

            set_transform(entity->transform * make_transform_matrix(entity->sprite_offset));
            draw_sprite(entity->sprite, entity->sprite_size * entity->sprite->aspect, entity->sprite_size);

            #if DEBUG
                if (entity->has_collider) {
                    set_transform(entity->transform);
                    draw_circle(make_circle(entity->position, entity->collider_radius), make_color(0.0f, 1.0f, 0.0f), false);
                }
            #endif
        }

        switch (game_mode) {
            case Game_Mode::MENU: {
                update_menu();
                break;
            }
            case Game_Mode::PLAY: {
                update_play();
                break;
            }
            invalid_default_case();
        }

        #if DEBUG
            set_projection(gui_projection);

            Vector2 layout = make_vector2(16.0f, platform.window_height - 16.0f - get_text_height(&font_arial, 24.0f));
            set_transform(make_transform_matrix(layout));

            draw_text(&font_arial, 24.0f, format_string("%.2f, %.2f, %i", timers.now, timers.delta * 1000.0f, (u32) (1.0f / timers.delta)));
            
            layout.y -= get_text_height(&font_arial, 24.0f);
            set_transform(make_transform_matrix(layout));

            draw_text(&font_arial, 24.0f, format_string("%u, %u", platform.heap_memory_allocated, platform.temp_memory_allocated));
        #endif

        swap_buffers();
    }
    
    return 0;
}