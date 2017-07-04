#pragma warning(push)
    #pragma warning(disable: 4244)
    #pragma warning(disable: 4456)
    #pragma warning(disable: 4459)
    #pragma warning(disable: 4505)

    #define STB_IMAGE_IMPLEMENTATION
    #define STBI_ONLY_PNG

    #define STBI_MALLOC(sz)        heap_alloc((u32) (sz))
    #define STBI_REALLOC(p, newsz) heap_realloc(p, (u32) (newsz))
    #define STBI_FREE(p)           heap_dealloc(p)

    #define STBI_ASSERT(x) assert(x)

    #include "../lib/stb_image.h"

    #define STB_TRUETYPE_IMPLEMENTATION

    #define STBTT_malloc(x, u) ((void) (u), heap_alloc((u32) (x)))
    #define STBTT_free(x, u)   ((void) (u), heap_dealloc(x))

    #define STBTT_assert(x) assert(x)

    #include "../lib/stb_truetype.h"
#pragma warning(pop)

struct Color {
    f32 r = 0.0f;
    f32 g = 0.0;
    f32 b = 0.0f;
    f32 a = 1.0f;
};

Color make_color(f32 r, f32 g, f32 b, f32 a = 1.0f) {
    Color color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}

void set_projection(Matrix4 projection) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((f32*) &projection);
}

void set_transform(Matrix4 transform) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((f32*) &transform);
}

void draw_rectangle(Rectangle2 rectangle, Color color, bool fill = true) {
    set_transform(make_identity_matrix());

    glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
    glColor4f(color.r, color.g, color.b, color.a);

    glVertex2f(rectangle.x1, rectangle.y1);
    glVertex2f(rectangle.x2, rectangle.y1);
    glVertex2f(rectangle.x2, rectangle.y2);
    glVertex2f(rectangle.x1, rectangle.y2);

    glEnd();
}

void draw_circle(Circle circle, Color color, bool fill = true) {
    set_transform(make_identity_matrix());

    glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
    glColor4f(color.r, color.g, color.b, color.a);

    for (u32 i = 0; i < 360; i++) {
        f32 theta = to_radians((f32) i);
        
        glVertex2f(
            circle.position.x + (cosf(theta) * circle.radius), 
            circle.position.y + (sinf(theta) * circle.radius));
    }

    glEnd();
}

const utf32 CODEPOINT_START = 32;
const utf32 CODEPOINT_COUNT = 96;

const u32 BAKED_FONT_BITMAP_WIDTH  = 512;
const u32 BAKED_FONT_BITMAP_HEIGHT = 512;

struct Baked_Font {
    f32 size = 0.0f;
    u32 texture_id = 0;

    stbtt_bakedchar glyphs[CODEPOINT_COUNT];
};

struct Font {
    bool is_valid = false;

    utf8* file_name = null;
    void* ttf_data  = null;

    stbtt_fontinfo info;
    Array<Baked_Font> baked;
};

Font load_font(utf8* file_name) {
    Font font;
    font.file_name = file_name;

    font.ttf_data = read_entire_file(font.file_name);
    if (font.ttf_data) {
        stbtt_InitFont(&font.info, (u8*) font.ttf_data, 0);

        printf("Loaded font '%s'\n", font.file_name);
        font.is_valid = true;
    }
    else {
        printf("Failed to load font '%s'\n", font.file_name);
    }

    return font;
}

f32 get_text_width(Font* font, f32 size, utf8* text) {
    if (!font->is_valid) return 0.0f;

    f32 font_scale = stbtt_ScaleForPixelHeight(&font->info, size);

    f32 width = 0.0f;
    while (*text) {
        utf32 codepoint = *text;

        i32 advance_width, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font->info, codepoint, &advance_width, &left_side_bearing);

        width += advance_width * font_scale;
        
        utf32 next_codepoint = *(text + 1);
        if (next_codepoint) {
            f32 kern_advance = (f32) stbtt_GetCodepointKernAdvance(&font->info, codepoint, next_codepoint);
            width += kern_advance * font_scale;
        }

        text += 1;
    }

    return width;
}

f32 get_text_height(Font* font, f32 size) {
    if (!font->is_valid) return 0.0f;

    f32 font_scale = stbtt_ScaleForPixelHeight(&font->info, size);

    i32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);

    return (ascent - descent + line_gap) * font_scale;
}

void draw_text(Font* font, f32 size, utf8* text, Color color = make_color(1.0f, 1.0f, 1.0f)) {
    if (!font->is_valid) return;

    Baked_Font* baked_font = null;
    for_each (Baked_Font* it, &font->baked) {
        if (it->size != size) continue;

        baked_font = it;
        break;
    }

    if (!baked_font) {
        baked_font = next(&font->baked);
        baked_font->size = size;

        u32 temp_start = platform.temp_memory_allocated;
        void* bitmap = temp_alloc(BAKED_FONT_BITMAP_WIDTH * BAKED_FONT_BITMAP_HEIGHT);
        
        i32 result = stbtt_BakeFontBitmap(
            (u8*) font->ttf_data, 
            0, 
            size, 
            (u8*) bitmap, 
            BAKED_FONT_BITMAP_WIDTH, 
            BAKED_FONT_BITMAP_HEIGHT, 
            CODEPOINT_START, 
            CODEPOINT_COUNT, 
            baked_font->glyphs);

        assert(result > 0);

        glGenTextures(1, &baked_font->texture_id);
        glBindTexture(GL_TEXTURE_2D, baked_font->texture_id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, BAKED_FONT_BITMAP_WIDTH, BAKED_FONT_BITMAP_HEIGHT, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        platform.temp_memory_allocated = temp_start;
    }

    glBindTexture(GL_TEXTURE_2D, baked_font->texture_id);
    glBegin(GL_QUADS);

    glColor4f(color.r, color.g, color.b, color.a);

    f32 position_x = 0;
    f32 position_y = 0;

    utf8* cursor = text;
    while (*cursor) {
        utf32 codepoint = *cursor;
        if (CODEPOINT_START <= codepoint && codepoint < CODEPOINT_START + CODEPOINT_COUNT) {
            stbtt_aligned_quad baked_quad;
            
            stbtt_GetBakedQuad(
                baked_font->glyphs, 
                BAKED_FONT_BITMAP_WIDTH, 
                BAKED_FONT_BITMAP_HEIGHT, 
                codepoint - CODEPOINT_START, 
                &position_x, 
                &position_y, 
                &baked_quad, 
                1);

            glTexCoord2f(baked_quad.s0, baked_quad.t0);
            glVertex2f(baked_quad.x0, -baked_quad.y0);

            glTexCoord2f(baked_quad.s1, baked_quad.t0);
            glVertex2f(baked_quad.x1, -baked_quad.y0);

            glTexCoord2f(baked_quad.s1, baked_quad.t1);
            glVertex2f(baked_quad.x1, -baked_quad.y1);

            glTexCoord2f(baked_quad.s0, baked_quad.t1);
            glVertex2f(baked_quad.x0, -baked_quad.y1);
        }

        cursor += 1;
    }

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    // draw_rectangle(get_text_width(font, size, text), get_text_height(font, size), 0.0f, 1.0f, 0.0f, 1.0f, false, false);
}

struct Sprite {
    bool is_valid = false;

    u32 texture = 0;
    u32 width   = 0;
    u32 height  = 0;
    f32 aspect  = 0.0f;
};

Sprite load_sprite(utf8* file_name) {
    Sprite sprite;

    u8* image = stbi_load(file_name, (i32*) &sprite.width, (i32*) &sprite.height, null, 4);
    sprite.aspect = (f32) sprite.width / (f32) sprite.height;

    if (image) {
        glGenTextures(1, &sprite.texture);
        glBindTexture(GL_TEXTURE_2D, sprite.texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite.width, sprite.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);

        printf("Loaded sprite '%s'\n", file_name);
        sprite.is_valid = true;
    }
    else {
        printf("Failed to load sprite '%s'\n", file_name);
    }

    return sprite;
}

void draw_sprite(Sprite* sprite, f32 height, bool center = true) {
    if (!sprite->is_valid) {
        draw_rectangle(make_rectangle2(make_vector2(), height, height, center), make_color(1.0f, 1.0f, 1.0f), true);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glBegin(GL_QUADS);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    f32 x = 0.0f;
    f32 y = 0.0f;

    f32 width = height * sprite->aspect;

    if (center) {
        x -= width  / 2.0f;
        y -= height / 2.0f;
    }

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, y);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + width, y);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + width, y + height);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y + height);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

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

enum Ship_Type {
    SHIP_TYPE_1,
    SHIP_TYPE_2,
    SHIP_TYPE_3,
    SHIP_TYPE_COUNT
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

Sprite sprite_asteroids[ASTEROID_SIZE_COUNT][ASTEROID_TYPE_COUNT];
Sprite sprite_ships[SHIP_COLOR_COUNT][SHIP_TYPE_COUNT];
Sprite sprite_enemies[ENEMY_COLOR_COUNT];
Sprite sprite_lasers[LASER_COLOR_COUNT];

Sprite* get_asteroid_sprite(Asteroid_Size size) {
    assert(0 <= size && size < ASTEROID_SIZE_COUNT);
    return &sprite_asteroids[size][get_random_out_of(ASTEROID_TYPE_COUNT)];
}

Sprite* get_ship_sprite(Ship_Color color, Ship_Type type) {
    assert(0 <= color && color < SHIP_COLOR_COUNT);
    assert(0 <= type  && type  < SHIP_TYPE_COUNT);

    return &sprite_ships[color][type];
}

Sprite* get_enemy_sprite(Enemy_Color color) {
    assert(0 <= color && color < ENEMY_COLOR_COUNT);
    return &sprite_enemies[color];
}

Sprite* get_laser_sprite(Laser_Color color) {
    assert(0 <= color && color < LASER_COLOR_COUNT);
    return &sprite_lasers[color];
}

void init_draw() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    font_arial             = load_font("c:/windows/fonts/arial.ttf");
    font_future            = load_font("data/fonts/future.ttf");
    font_starjedi          = load_font("data/fonts/starjedi.ttf");
    font_moonhouse         = load_font("data/fonts/moonhouse.ttf");
    font_nasalization      = load_font("data/fonts/nasalization-rg.ttf");

    sprite_background = load_sprite("data/sprites/background.png");
    sprite_ui_ship    = load_sprite("data/sprites/ui_ship.png");
    sprite_thrust     = load_sprite("data/sprites/thrust.png");
    sprite_shield     = load_sprite("data/sprites/shield.png");

    sprite_asteroids[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_1]  = load_sprite("data/sprites/asteroid_small_01.png");
    sprite_asteroids[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_2]  = load_sprite("data/sprites/asteroid_small_02.png");
    sprite_asteroids[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_3]  = load_sprite("data/sprites/asteroid_small_03.png");
    sprite_asteroids[ASTEROID_SIZE_SMALL][ASTEROID_TYPE_4]  = load_sprite("data/sprites/asteroid_small_04.png");
    sprite_asteroids[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_1] = load_sprite("data/sprites/asteroid_medium_01.png");
    sprite_asteroids[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_2] = load_sprite("data/sprites/asteroid_medium_02.png");
    sprite_asteroids[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_3] = load_sprite("data/sprites/asteroid_medium_03.png");
    sprite_asteroids[ASTEROID_SIZE_MEDIUM][ASTEROID_TYPE_4] = load_sprite("data/sprites/asteroid_medium_04.png");
    sprite_asteroids[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_1]  = load_sprite("data/sprites/asteroid_large_01.png");
    sprite_asteroids[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_2]  = load_sprite("data/sprites/asteroid_large_02.png");
    sprite_asteroids[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_3]  = load_sprite("data/sprites/asteroid_large_03.png");
    sprite_asteroids[ASTEROID_SIZE_LARGE][ASTEROID_TYPE_4]  = load_sprite("data/sprites/asteroid_large_04.png");

    sprite_ships[SHIP_COLOR_RED][SHIP_TYPE_1]    = load_sprite("data/sprites/ship_red_01.png");
    sprite_ships[SHIP_COLOR_RED][SHIP_TYPE_2]    = load_sprite("data/sprites/ship_red_02.png");
    sprite_ships[SHIP_COLOR_RED][SHIP_TYPE_3]    = load_sprite("data/sprites/ship_red_03.png");
    sprite_ships[SHIP_COLOR_GREEN][SHIP_TYPE_1]  = load_sprite("data/sprites/ship_green_01.png");
    sprite_ships[SHIP_COLOR_GREEN][SHIP_TYPE_2]  = load_sprite("data/sprites/ship_green_02.png");
    sprite_ships[SHIP_COLOR_GREEN][SHIP_TYPE_3]  = load_sprite("data/sprites/ship_green_03.png");
    sprite_ships[SHIP_COLOR_BLUE][SHIP_TYPE_1]   = load_sprite("data/sprites/ship_blue_01.png");
    sprite_ships[SHIP_COLOR_BLUE][SHIP_TYPE_2]   = load_sprite("data/sprites/ship_blue_02.png");
    sprite_ships[SHIP_COLOR_BLUE][SHIP_TYPE_3]   = load_sprite("data/sprites/ship_blue_03.png");
    sprite_ships[SHIP_COLOR_ORANGE][SHIP_TYPE_1] = load_sprite("data/sprites/ship_orange_01.png");
    sprite_ships[SHIP_COLOR_ORANGE][SHIP_TYPE_2] = load_sprite("data/sprites/ship_orange_02.png");
    sprite_ships[SHIP_COLOR_ORANGE][SHIP_TYPE_3] = load_sprite("data/sprites/ship_orange_03.png");

    sprite_enemies[ENEMY_COLOR_YELLOW] = load_sprite("data/sprites/enemy_yellow.png");
    sprite_enemies[ENEMY_COLOR_ORANGE] = load_sprite("data/sprites/enemy_orange.png");
    
    sprite_lasers[LASER_COLOR_RED]  = load_sprite("data/sprites/laser_red.png");
    sprite_lasers[LASER_COLOR_BLUE] = load_sprite("data/sprites/laser_blue.png");
}