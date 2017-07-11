#include "platform.cpp"
#include "math.cpp"
#include "data_structures.cpp"

Allocator heap_allocator;
Allocator temp_allocator;

#include "draw.cpp"
#include "sound.cpp"
#include "assets.cpp"
#include "gui.cpp"

Playing_Sound* playing_music;
f32 music_volume;

f32 world_height;
f32 world_width;

f32 world_left;
f32 world_right;
f32 world_top;
f32 world_bottom;

Matrix4 world_projection;

Vector2 get_world_position(i32 screen_x, i32 screen_y) {
    return unproject(screen_x, screen_y, platform.window_width, platform.window_height, world_projection);
}

#include "particles.cpp"
#include "entities.cpp"

enum Game_Mode {
    GAME_MODE_NONE,
    GAME_MODE_MENU,
    GAME_MODE_SURVIVAL
};

utf8* to_string(Game_Mode game_mode) {
    #define case(type) case type: return #type

    switch (game_mode) {
        case(GAME_MODE_NONE);
        case(GAME_MODE_MENU);
        case(GAME_MODE_SURVIVAL);
    }

    #undef case

    return "INVALID";
}

Game_Mode game_mode;
void switch_game_mode(Game_Mode new_game_mode);

#include "game_modes/menu.cpp"
#include "game_modes/survival.cpp"

void switch_game_mode(Game_Mode new_game_mode) {
    switch (game_mode) {
        case GAME_MODE_NONE: {
            break;
        }
        case GAME_MODE_MENU: {
            stop_menu();
            break;
        }
        case GAME_MODE_SURVIVAL: {
            stop_survival();
            break;
        }
        invalid_default_case();
    }

    switch (new_game_mode) {
        case GAME_MODE_MENU: {
            start_menu();
            break;
        }
        case GAME_MODE_SURVIVAL: {
            start_survival();
            break;
        }
        invalid_default_case();
    }

    printf("Game mode switched from '%s' to '%s'\n", to_string(game_mode), to_string(new_game_mode));
    game_mode = new_game_mode;
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

        if (should_simulate) {
            update_entities();
            update_particles();
        }

        set_projection(world_projection);

        f32 tile_size = 5.0f;
        
        u32 tiles_x = (u32) (world_width  / tile_size) + 1;
        u32 tiles_y = (u32) (world_height / tile_size) + 1;
        
        for (u32 x = 0; x < tiles_x; x++) {
            for (u32 y = 0; y < tiles_y; y++) {
                Vector2 position = make_vector2(world_left, world_bottom);
                position += make_vector2((f32) x, (f32) y) * tile_size;

                set_transform(make_transform_matrix(position));
                draw_sprite(&sprite_background, tile_size, 1.0f, false);
            }
        }

        draw_entities();
        draw_particles();

        switch (game_mode) {
            case GAME_MODE_MENU: {
                update_menu();
                break;
            }
            case GAME_MODE_SURVIVAL: {
                update_survival();
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
                    gui_text(&font_arial, format_string("Heap: %.2f mb", platform.heap_memory_allocated / 1024.0f), 18.0f);
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