#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../lib/stb_truetype.h"

float font_height;
float font_vertical_advance;

stbtt_bakedchar font_glyphs[96];
GLuint          font_texture;

struct Sprite {
    GLuint texture = 0;

    int width  = 0;
    int height = 0;
};

Sprite load_sprite(char* file_name) {
    Sprite sprite;

    uint8_t* image = stbi_load(file_name, &sprite.width, &sprite.height, NULL, 4);
    assert(image);

    glGenTextures(1, &sprite.texture);
    glBindTexture(GL_TEXTURE_2D, sprite.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite.width, sprite.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    return sprite;
}

void draw_sprite(Sprite* sprite, bool center = true) {
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glBegin(GL_QUADS);

    float height = 1.0f;
    float width  = height * ((float) sprite->width / (float) sprite->height);

    float x = 0.0f;
    float y = 0.0f;

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

Sprite background_sprite;
Sprite ship_sprite;

void init_draw() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    void* ttf_file = read_entire_file("c:/windows/fonts/arial.ttf");
    assert(ttf_file);

    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, (uint8_t*) ttf_file, 0);

    float font_scale = stbtt_ScaleForPixelHeight(&font_info, 18.0f);

    int ascent, descent, line_gap;
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

    background_sprite = load_sprite("data/sprites/background.png");
    ship_sprite       = load_sprite("data/sprites/player_ship.png");
}

void set_projection(Matrix4* projection) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((float*) projection);
}

void set_transform(Matrix4* transform) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((float*) transform);
}

void draw_rectangle(float width, float height, float r, float g, float b, float a) {
    glBegin(GL_QUADS);
    glColor4f(r, g, b, a);

    float half_width  = width  / 2.0f;
    float half_height = height / 2.0f;

    glVertex2f(-half_width, -half_height);
    glVertex2f( half_width, -half_height);
    glVertex2f( half_width,  half_height);
    glVertex2f(-half_width,  half_height);

    glEnd();
}

void draw_text(char* string, ...) {
    va_list args;
    va_start(args, string);

    string = format_string_args(string, args);
    va_end(args);

    glBindTexture(GL_TEXTURE_2D, font_texture);
    glBegin(GL_QUADS);

    float offset_x = 0.0f;
    float offset_y = 0.0f;

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