#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../lib/stb_truetype.h"

// struct Font {
//     utf8* file_name = null;
//     void* tff_data  = null;

//     stbtt_fontinfo info;
// };

// Font load_font(utf8* file_name) {
//     Font font;
//     font.file_name = file_name;

//     font.ttf_data = read_entire_file(font.file_name);


//     printf("Loaded font '%s'\n", font.file_name);
//     return font;
// }



f32 font_height;
f32 font_vertical_advance;

stbtt_bakedchar font_glyphs[96];
GLuint          font_texture;

struct Sprite {
    GLuint texture = 0;

    u32 width  = 0;
    u32 height = 0;
    f32 aspect = 0.0f;
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

    void* ttf_file = read_entire_file("c:/windows/fonts/arial.ttf");
    assert(ttf_file);

    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, (uint8_t*) ttf_file, 0);

    f32 font_scale = stbtt_ScaleForPixelHeight(&font_info, 18.0f);

    i32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

    font_height = (ascent - descent) * font_scale;
    font_vertical_advance = (ascent - descent + line_gap) * font_scale;

    uint8_t font_bitmap[512 * 512];
    stbtt_BakeFontBitmap((uint8_t*) ttf_file, 0, 18.0f, font_bitmap, 512, 512, 32, 96, font_glyphs);

    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    free(ttf_file);

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

void set_projection(Matrix4* projection) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((f32*) projection);
}

void set_transform(Matrix4* transform) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((f32*) transform);
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

void draw_text(utf8* string, ...) {
    if (is_using_layout) {

        Matrix4 transform = make_transform_matrix(layout_position);
        set_transform(&transform);

        layout_position.y -= font_vertical_advance;
    }

    va_list args;
    va_start(args, string);

    string = format_string_args(string, args);
    va_end(args);

    glBindTexture(GL_TEXTURE_2D, font_texture);
    glBegin(GL_QUADS);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    f32 offset_x = 0.0f;
    f32 offset_y = 0.0f;

    while (*string) {
        if (32 <= *string && *string < 128) {
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(font_glyphs, 512, 512, *string - 32, &offset_x, &offset_y, &quad, 1);

            glTexCoord2f(quad.s0, quad.t0);
            glVertex2f(quad.x0, -quad.y0);

            glTexCoord2f(quad.s1, quad.t0);
            glVertex2f(quad.x1, -quad.y0);

            glTexCoord2f(quad.s1, quad.t1);
            glVertex2f(quad.x1, -quad.y1);

            glTexCoord2f(quad.s0, quad.t1);
            glVertex2f(quad.x0, -quad.y1);
        }

        string += 1;
    }

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
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