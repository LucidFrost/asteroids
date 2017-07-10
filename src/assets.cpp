Font font_arial;
Font font_future;
Font font_starjedi;
Font font_moonhouse;
Font font_nasalization;

enum Asteroid_Size {
    ASTEROID_SIZE_SMALL,
    ASTEROID_SIZE_MEDIUM,
    ASTEROID_SIZE_LARGE,
    ASTEROID_SIZE_COUNT
};

enum Asteroid_Type {
    ASTEROID_TYPE_1,
    ASTEROID_TYPE_2,
    ASTEROID_TYPE_3,
    ASTEROID_TYPE_4,
    ASTEROID_TYPE_COUNT
};

enum Ship_Color {
    SHIP_COLOR_RED,
    SHIP_COLOR_GREEN,
    SHIP_COLOR_BLUE,
    SHIP_COLOR_ORANGE,
    SHIP_COLOR_COUNT
};

utf8* to_string(Ship_Color ship_color) {
    switch (ship_color) {
        case SHIP_COLOR_RED:    return "Red";
        case SHIP_COLOR_GREEN:  return "Green";
        case SHIP_COLOR_BLUE:   return "Blue";
        case SHIP_COLOR_ORANGE: return "Orange";
    }

    return "Invalid";
}

enum Ship_Type {
    SHIP_TYPE_1,
    SHIP_TYPE_2,
    SHIP_TYPE_3,
    SHIP_TYPE_COUNT
};

utf8* to_string(Ship_Type ship_type) {
    switch (ship_type) {
        case SHIP_TYPE_1: return "Type 1";
        case SHIP_TYPE_2: return "Type 2";
        case SHIP_TYPE_3: return "Type 3";
    }

    return "Invalid";
}

enum Damage_Type {
    DAMAGE_TYPE_SMALL,
    DAMAGE_TYPE_MEDIUM,
    DAMAGE_TYPE_LARGE,
    DAMAGE_TYPE_COUNT
};

enum Enemy_Color {
    ENEMY_COLOR_YELLOW,
    ENEMY_COLOR_ORANGE,
    ENEMY_COLOR_COUNT
};

enum Laser_Color {
    LASER_COLOR_RED,
    LASER_COLOR_BLUE,
    LASER_COLOR_COUNT
};

Sprite sprite_background;
Sprite sprite_ui_ship;
Sprite sprite_thrust;
Sprite sprite_shield;
Sprite sprite_asteroid[ASTEROID_SIZE_COUNT][ASTEROID_TYPE_COUNT];
Sprite sprite_ship[SHIP_COLOR_COUNT][SHIP_TYPE_COUNT];
Sprite sprite_damage[DAMAGE_TYPE_COUNT][SHIP_TYPE_COUNT];
Sprite sprite_enemy[ENEMY_COLOR_COUNT];
Sprite sprite_laser[LASER_COLOR_COUNT];

Sprite* get_asteroid_sprite(Asteroid_Size size) {
    return &sprite_asteroid[size][get_random_out_of(ASTEROID_TYPE_COUNT)];
}

Sprite* get_ship_sprite(Ship_Type type, Ship_Color color) {
    return &sprite_ship[color][type];
}

Sprite* get_damage_sprite(Ship_Type ship_type, Damage_Type damage_type) {
    return &sprite_damage[damage_type][ship_type];
}

Sprite* get_enemy_sprite(Enemy_Color color) {
    return &sprite_enemy[color];
}

Sprite* get_laser_sprite(Laser_Color color) {
    return &sprite_laser[color];
}

Sound sound_music;
Sound sound_spawn;
Sound sound_kill[2];
Sound sound_laser[2];
Sound sound_enemy_fire[2];

Sound* get_kill_sound() {
    return &sound_kill[get_random_out_of(count_of(sound_kill))];
}

Sound* get_laser_sound() {
    return &sound_laser[get_random_out_of(count_of(sound_laser))];
}

Sound* get_enemy_fire_sound() {
    return &sound_enemy_fire[get_random_out_of(count_of(sound_enemy_fire))];
}

void load_assets() {
    font_arial        = load_font("c:/windows/fonts/arial.ttf");
    font_future       = load_font("fonts/future.ttf");
    font_starjedi     = load_font("fonts/starjedi.ttf");
    font_moonhouse    = load_font("fonts/moonhouse.ttf");
    font_nasalization = load_font("fonts/nasalization-rg.ttf");

    sprite_background                                      = load_sprite("sprites/background.png");
    sprite_ui_ship                                         = load_sprite("sprites/ui_ship.png");
    sprite_thrust                                          = load_sprite("sprites/thrust.png");
    sprite_shield                                          = load_sprite("sprites/shield.png");
    sprite_asteroid[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_1]  = load_sprite("sprites/asteroid_small_01.png");
    sprite_asteroid[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_2]  = load_sprite("sprites/asteroid_small_02.png");
    sprite_asteroid[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_3]  = load_sprite("sprites/asteroid_small_03.png");
    sprite_asteroid[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_4]  = load_sprite("sprites/asteroid_small_04.png");
    sprite_asteroid[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_1] = load_sprite("sprites/asteroid_medium_01.png");
    sprite_asteroid[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_2] = load_sprite("sprites/asteroid_medium_02.png");
    sprite_asteroid[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_3] = load_sprite("sprites/asteroid_medium_03.png");
    sprite_asteroid[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_4] = load_sprite("sprites/asteroid_medium_04.png");
    sprite_asteroid[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_1]  = load_sprite("sprites/asteroid_large_01.png");
    sprite_asteroid[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_2]  = load_sprite("sprites/asteroid_large_02.png");
    sprite_asteroid[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_3]  = load_sprite("sprites/asteroid_large_03.png");
    sprite_asteroid[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_4]  = load_sprite("sprites/asteroid_large_04.png");
    sprite_ship[SHIP_COLOR_RED][SHIP_TYPE_1]               = load_sprite("sprites/ship_red_01.png");
    sprite_ship[SHIP_COLOR_RED][SHIP_TYPE_2]               = load_sprite("sprites/ship_red_02.png");
    sprite_ship[SHIP_COLOR_RED][SHIP_TYPE_3]               = load_sprite("sprites/ship_red_03.png");
    sprite_ship[SHIP_COLOR_GREEN][SHIP_TYPE_1]             = load_sprite("sprites/ship_green_01.png");
    sprite_ship[SHIP_COLOR_GREEN][SHIP_TYPE_2]             = load_sprite("sprites/ship_green_02.png");
    sprite_ship[SHIP_COLOR_GREEN][SHIP_TYPE_3]             = load_sprite("sprites/ship_green_03.png");
    sprite_ship[SHIP_COLOR_BLUE][SHIP_TYPE_1]              = load_sprite("sprites/ship_blue_01.png");
    sprite_ship[SHIP_COLOR_BLUE][SHIP_TYPE_2]              = load_sprite("sprites/ship_blue_02.png");
    sprite_ship[SHIP_COLOR_BLUE][SHIP_TYPE_3]              = load_sprite("sprites/ship_blue_03.png");
    sprite_ship[SHIP_COLOR_ORANGE][SHIP_TYPE_1]            = load_sprite("sprites/ship_orange_01.png");
    sprite_ship[SHIP_COLOR_ORANGE][SHIP_TYPE_2]            = load_sprite("sprites/ship_orange_02.png");
    sprite_ship[SHIP_COLOR_ORANGE][SHIP_TYPE_3]            = load_sprite("sprites/ship_orange_03.png");
    sprite_damage[DAMAGE_TYPE_SMALL][SHIP_TYPE_1]          = load_sprite("sprites/ship_damage_small_01.png");
    sprite_damage[DAMAGE_TYPE_SMALL][SHIP_TYPE_2]          = load_sprite("sprites/ship_damage_small_02.png");
    sprite_damage[DAMAGE_TYPE_SMALL][SHIP_TYPE_3]          = load_sprite("sprites/ship_damage_small_03.png");
    sprite_damage[DAMAGE_TYPE_MEDIUM][SHIP_TYPE_1]         = load_sprite("sprites/ship_damage_medium_01.png");
    sprite_damage[DAMAGE_TYPE_MEDIUM][SHIP_TYPE_2]         = load_sprite("sprites/ship_damage_medium_02.png");
    sprite_damage[DAMAGE_TYPE_MEDIUM][SHIP_TYPE_3]         = load_sprite("sprites/ship_damage_medium_03.png");
    sprite_damage[DAMAGE_TYPE_LARGE][SHIP_TYPE_1]          = load_sprite("sprites/ship_damage_large_01.png");
    sprite_damage[DAMAGE_TYPE_LARGE][SHIP_TYPE_2]          = load_sprite("sprites/ship_damage_large_02.png");
    sprite_damage[DAMAGE_TYPE_LARGE][SHIP_TYPE_3]          = load_sprite("sprites/ship_damage_large_03.png");
    sprite_enemy[ENEMY_COLOR_YELLOW]                       = load_sprite("sprites/enemy_yellow.png");
    sprite_enemy[ENEMY_COLOR_ORANGE]                       = load_sprite("sprites/enemy_orange.png");
    sprite_laser[LASER_COLOR_RED]                          = load_sprite("sprites/laser_red.png");
    sprite_laser[LASER_COLOR_BLUE]                         = load_sprite("sprites/laser_blue.png");

    sound_music = load_sound("sounds/music.ogg");
    sound_spawn = load_sound("sounds/spawn.ogg");
    
    for (u32 i = 0; i < count_of(sound_kill); i++) {
        sound_kill[i] = load_sound(format_string("sounds/kill_%02u.ogg", i + 1));
    }

    for (u32 i = 0; i < count_of(sound_laser); i++) {
        sound_laser[i] = load_sound(format_string("sounds/laser_%02u.ogg", i + 1));
    }

    for (u32 i = 0; i < count_of(sound_enemy_fire); i++) {
        sound_enemy_fire[i] = load_sound(format_string("sounds/enemy_fire_%02u.ogg", i + 1));
    }
}