#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../lib/stb_truetype.h"

void set_projection(Matrix4* projection) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((f32*) projection);
}

void set_transform(Matrix4* transform) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((f32*) transform);
}

void set_transform(Vector2 position, f32 orientation = 0.0, f32 scale = 1.0f) {
    Matrix4 transform = make_transform_matrix(position, orientation, scale);
    set_transform(&transform);
}

struct Glyph {
    utf32 codepoint = 0;
    f32   size      = 0.0f;
        
    f32 x1 = 0.0f;
    f32 y1 = 0.0f;
    f32 x2 = 0.0f;
    f32 y2 = 0.0f;

    u32 texture_id = 0;
};

struct Font {
    utf8* file_name = null;
    void* ttf_data  = null;

    stbtt_fontinfo info;
    Array<Glyph> glyphs;
};

Font load_font(utf8* file_name) {
    Font font;
    font.file_name = file_name;

    font.ttf_data = read_entire_file(font.file_name);
    assert(font.ttf_data);

    stbtt_InitFont(&font.info, (u8*) font.ttf_data, 0);

    printf("Loaded font '%s'\n", font.file_name);
    return font;
}

Vector2 layout_position;
bool is_using_layout;

void begin_layout(f32 x, f32 y) {
    assert(!is_using_layout);

    layout_position = make_vector2(x, y);
    is_using_layout = true;
}

void end_layout() {
    assert(is_using_layout);
    is_using_layout = false;
}

f32 get_text_width(Font* font, f32 size, utf8* text) {
    f32 font_scale = stbtt_ScaleForPixelHeight(&font->info, size);

    f32 width = 0.0f;
    while (*text) {
        utf32 codepoint = *text;

        i32 advance_width, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font->info, codepoint, &advance_width, &left_side_bearing);

        width += advance_width * font_scale;
        
        utf32 next_codepoint = *(text + 1);
        if (next_codepoint) {
            f32 kern_advance = stbtt_GetCodepointKernAdvance(&font->info, codepoint, next_codepoint);
            width += kern_advance * font_scale;
        }

        text += 1;
    }

    return width;
}

f32 get_text_height(Font* font, f32 size) {
    f32 font_scale = stbtt_ScaleForPixelHeight(&font->info, size);

    i32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);

    return (ascent - descent + line_gap) * font_scale;
}

void draw_text(Font* font, f32 size, utf8* text, f32 r, f32 g, f32 b, f32 a) {
    f32 font_scale = stbtt_ScaleForPixelHeight(&font->info, size);

    if (is_using_layout) {
        set_transform(layout_position);
        layout_position.y -= get_text_height(font, size);
    }
    
    f32 position_x = 0.0f;
    f32 position_y = 0.0f;

    while (*text) {
        utf32 codepoint = *text;

        if (codepoint != ' ') {
            Glyph* glyph = null;
            for_each (Glyph* it, &font->glyphs) {
                if (it->codepoint != codepoint) continue;
                if (it->size != size)           continue;

                glyph = it;
                break;
            }

            if (!glyph) {
                glyph = next(&font->glyphs);

                glyph->codepoint = codepoint;
                glyph->size      = size;

                i32 x1, y1, x2, y2;
                stbtt_GetCodepointBitmapBox(&font->info, codepoint, font_scale, font_scale, &x1, &y1, &x2, &y2);

                glyph->x1 = x1;
                glyph->y1 = y1;
                glyph->x2 = x2;
                glyph->y2 = y2;

                u32 width  = x2 - x1;
                u32 height = y2 - y1;

                void* bitmap = temp_alloc(width * height);
                stbtt_MakeCodepointBitmap(&font->info, (u8*) bitmap, width, height, width, font_scale, font_scale, codepoint);

                glGenTextures(1, &glyph->texture_id);

                glBindTexture(GL_TEXTURE_2D, glyph->texture_id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            glBindTexture(GL_TEXTURE_2D, glyph->texture_id);
            glBegin(GL_QUADS);

            glColor4f(r, g, b, a);

            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(position_x + glyph->x1, -(position_y + glyph->y1));

            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(position_x + glyph->x2, -(position_y + glyph->y1));

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(position_x + glyph->x2, -(position_y + glyph->y2));

            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(position_x + glyph->x1, -(position_y + glyph->y2));

            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        i32 advance_width, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font->info, codepoint, &advance_width, &left_side_bearing);

        position_x += advance_width * font_scale;
        
        utf32 next_codepoint = *(text + 1);
        if (next_codepoint) {
            f32 kern_advance = stbtt_GetCodepointKernAdvance(&font->info, codepoint, next_codepoint);
            position_x += kern_advance * font_scale;
        }
        
        text += 1;
    }
}

struct Sprite {
    u32 texture = 0;
    u32 width   = 0;
    u32 height  = 0;
    f32 aspect  = 0.0f;
};

Sprite load_sprite(utf8* file_name) {
    Sprite sprite;

    u8* image = stbi_load(file_name, (i32*) &sprite.width, (i32*) &sprite.height, null, 4);
    assert(image);

    sprite.aspect = (f32) sprite.width / (f32) sprite.height;

    glGenTextures(1, &sprite.texture);
    glBindTexture(GL_TEXTURE_2D, sprite.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite.width, sprite.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    printf("Loaded sprite '%s'\n", file_name);
    return sprite;
}

void draw_sprite(Sprite* sprite, f32 width, f32 height, bool center = true) {
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glBegin(GL_QUADS);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    f32 x = 0.0f;
    f32 y = 0.0f;

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

void draw_rectangle(f32 width, f32 height, f32 r, f32 g, f32 b, f32 a, bool center = true, bool fill = true) {
    glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
    glColor4f(r, g, b, a);

    f32 x = 0.0f;
    f32 y = 0.0f;

    if (center) {
        x -= width  / 2.0f;
        y -= height / 2.0f;
    }

    glVertex2f(x,         y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x,         y + height);

    glEnd();
}

void draw_circle(f32 radius, f32 r, f32 g, f32 b, f32 a, bool fill = true) {
    glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
    glColor4f(r, g, b, a);

    for (u32 i = 0; i < 360; i++) {
        f32 theta = to_radians((f32) i);
        glVertex2f(cosf(theta) * radius, sinf(theta) * radius);
    }

    glEnd();
}

Font font_arial;
Font font_future;

Sprite background_sprite;
Sprite ship_sprite;
Sprite laser_sprite;
Sprite thrust_sprite;
Sprite asteroid_small_sprite;
Sprite asteroid_medium_sprite;
Sprite asteroid_large_sprite;
Sprite enemy_big_sprite;
Sprite enemy_small_sprite;
Sprite player_life_sprite;

void init_draw() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    font_arial  = load_font("c:/windows/fonts/arial.ttf");
    font_future = load_font("data/fonts/future.ttf");

    background_sprite      = load_sprite("data/sprites/background.png");
    ship_sprite            = load_sprite("data/sprites/ship.png");
    laser_sprite           = load_sprite("data/sprites/laser.png");
    thrust_sprite          = load_sprite("data/sprites/thrust.png");
    asteroid_small_sprite  = load_sprite("data/sprites/asteroid_small.png");
    asteroid_medium_sprite = load_sprite("data/sprites/asteroid_medium.png");
    asteroid_large_sprite  = load_sprite("data/sprites/asteroid_large.png");
    enemy_big_sprite       = load_sprite("data/sprites/enemy_big.png");
    enemy_small_sprite     = load_sprite("data/sprites/enemy_small.png");
    player_life_sprite     = load_sprite("data/sprites/player_life.png");
}