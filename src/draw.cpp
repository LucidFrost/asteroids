#include <gl/gl.h>

#include "../lib/glext.h"
#include "../lib/wglext.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../lib/stb_truetype.h"

#pragma comment(lib, "opengl32.lib")

const float PIXELS_TO_WORLD = 1.0f / 100.0f;

struct Color {
    float r;
    float g;
    float b;
    float a;
};

Color make_color(float r, float g, float b, float a = 1.0f) {
    Color color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}

const Color BLACK = make_color(0.0f, 0.0f, 0.0f);
const Color WHITE = make_color(1.0f, 1.0f, 1.0f);
const Color RED   = make_color(1.0f, 0.0f, 0.0f);
const Color GREEN = make_color(0.0f, 1.0f, 0.0f);
const Color BLUE  = make_color(0.0f, 0.0f, 1.0f);

HDC dc;

PFNGLGENBUFFERSPROC                 glGenBuffers;
PFNGLBINDBUFFERPROC                 glBindBuffer;
PFNGLBUFFERDATAPROC                 glBufferData;
PFNGLBUFFERSUBDATAPROC              glBufferSubData;
PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray;

PFNGLCREATEPROGRAMPROC              glCreateProgram;
PFNGLCREATESHADERPROC               glCreateShader;
PFNGLDELETESHADERPROC               glDeleteShader;
PFNGLSHADERSOURCEPROC               glShaderSource;
PFNGLGETSHADERIVPROC                glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog;
PFNGLCOMPILESHADERPROC              glCompileShader;
PFNGLATTACHSHADERPROC               glAttachShader;
PFNGLDETACHSHADERPROC               glDetachShader;
PFNGLLINKPROGRAMPROC                glLinkProgram;
PFNGLGETPROGRAMIVPROC               glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC                 glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation;
PFNGLUNIFORM4FPROC                  glUniform4f;
PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv;

GLuint vertex_buffer;
GLuint font_texture;

GLuint simple_program;
GLuint sprite_program;
GLuint font_program;

uint8_t font_bitmap[512 * 512];
stbtt_bakedchar font_glyphs[96];

Matrix4 world_projection;
Matrix4 gui_projection;

void check_gl_error(char* identifier) {
    for (;;) {
        GLenum error = glGetError();
        if (error == GL_NO_ERROR) break;

        char* error_string = NULL;
        switch (error) {
            case GL_INVALID_OPERATION: error_string = "INVALID_OPERATION"; break;
            case GL_INVALID_ENUM:      error_string = "INVALID_ENUM";      break;
            case GL_INVALID_VALUE:     error_string = "INVALID_VALUE";     break;
            case GL_OUT_OF_MEMORY:     error_string = "OUT_OF_MEMORY";     break;
            default:                   error_string = "UNKNOWN";           break;
        }

        print("%s: OpenGL error %s (%i)\n", identifier, error_string, error);
    }
}

GLuint compile_shader(GLenum type, char* source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        print("Shader compilation failed:\n");

        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        GLchar* buffer = (GLchar*) alloc(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, buffer);

        print(buffer);
        print("\n");

        dealloc(buffer);
    }

    return shader;
}

GLuint make_program(char* vertex_source, char* fragment_source) {
    GLuint program = glCreateProgram();

    GLuint vertex_shader   = compile_shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (!status) {
        print("Program link failed:\n");

        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        GLchar* buffer = (GLchar*) alloc(log_length);
        glGetProgramInfoLog(program, log_length, NULL, buffer);

        print(buffer);
        print("\n");

        dealloc(buffer);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void init_draw(HWND window) {
    dc = GetDC(window);

    PIXELFORMATDESCRIPTOR pixel_format_description = {};
    pixel_format_description.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format_description.nVersion = 1;
    pixel_format_description.dwFlags  = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    int pixel_format = ChoosePixelFormat(dc, &pixel_format_description);
    SetPixelFormat(dc, pixel_format, &pixel_format_description);

    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);

    glGenBuffers                = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
    glBindBuffer                = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
    glBufferData                = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
    glBufferSubData             = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
    glVertexAttribPointer       = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray   = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");

    glCreateProgram             = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
    glCreateShader              = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
    glDeleteShader              = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
    glShaderSource              = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
    glGetShaderiv               = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog          = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
    glCompileShader             = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
    glAttachShader              = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
    glDetachShader              = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
    glLinkProgram               = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
    glGetProgramiv              = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog         = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
    glUseProgram                = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    glGetUniformLocation        = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
    glUniform4f                 = (PFNGLUNIFORM4FPROC) wglGetProcAddress("glUniform4f");
    glUniformMatrix4fv          = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 512 * 1024, NULL, GL_STREAM_DRAW);

    simple_program = make_program(
        R"(
            #version 430 core

            uniform mat4 projection;

            layout(location = 0) in vec2 position;

            void main() {
                gl_Position = projection * vec4(position, 0.0f, 1.0f);
            }
        )",
        R"(
            #version 430 core

            uniform vec4 color;

            out vec4 fragment_color;

            void main() {
                fragment_color = color;
            }
        )");

    sprite_program = make_program(
        R"(
            #version 430 core

            uniform mat4 projection;
            uniform mat4 transform;

            layout(location = 0) in vec2 position;
            layout(location = 1) in vec2 uv;

            out vec2 fragment_uv;

            void main() {
                gl_Position = projection * transform * vec4(position, 0.0f, 1.0f);
                fragment_uv = uv;
            }
        )",
        R"(
            #version 430 core

            uniform sampler2D sprite;

            in  vec2 fragment_uv;
            out vec4 fragment_color;

            void main() {
                fragment_color = texture(sprite, fragment_uv);
            }
        )");

    font_program = make_program(
        R"(
            #version 430 core

            uniform mat4 projection;
            
            layout(location = 0) in vec2 position;
            layout(location = 1) in vec2 uv;

            out vec2 fragment_uv;

            void main() {
                gl_Position = projection * vec4(position, 0.0f, 1.0f);
                fragment_uv = uv;
            }
        )", 
        R"(
            #version 430 core

            uniform sampler2D font_sheet;
            uniform vec4      font_color;

            in  vec2 fragment_uv;
            out vec4 fragment_color;

            void main() {
                fragment_color = font_color * vec4(1.0f, 1.0f, 1.0f, texture(font_sheet, fragment_uv).r);
            }
        )");

    uint8_t* font_data = (uint8_t*) read_entire_file("data/future.ttf");
    if (font_data) {
        stbtt_BakeFontBitmap(font_data, 0, 24.0f, font_bitmap, 512, 512, 32, 96, font_glyphs);

        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, font_bitmap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
    check_gl_error("init_draw");

    world_projection = make_ortho(-HALF_WORLD_WIDTH, HALF_WORLD_WIDTH, HALF_WORLD_HEIGHT, -HALF_WORLD_HEIGHT);
    gui_projection   = make_ortho(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);
}

struct Sprite {
    GLuint texture;
    
    int width;
    int height;
};

Sprite load_sprite(char* file_name) {
    Sprite sprite = {};

    uint8_t* data = stbi_load(file_name, &sprite.width, &sprite.height, NULL, 4);
    if (data) {
        glGenTextures(1, &sprite.texture);
        glBindTexture(GL_TEXTURE_2D, sprite.texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite.width, sprite.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        print("Failed to load sprite '%s'\n", file_name);
    }

    return sprite;
}

void draw_begin() {
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void draw_end() {
    SwapBuffers(dc);
}

void draw_rectangle(Vector2 position, float width, float height, Color color) {
    glUseProgram(simple_program);

    GLint projection_location = glGetUniformLocation(simple_program, "projection");
    if (projection_location == -1) {
        print("Simple program uniform 'projection' was not found\n");
    }
    else {
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, &world_projection._11);
    }

    GLint color_location = glGetUniformLocation(simple_program, "color");
    if (color_location == -1) {
        print("Simple program uniform 'color' was not found\n");
    }
    else {
        glUniform4f(color_location, color.r, color.g, color.b, color.a);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    Vector2 positions[] = {
        make_vector2(position.x,         position.y),
        make_vector2(position.x + width, position.y),
        make_vector2(position.x + width, position.y + height),
        make_vector2(position.x,         position.y + height)
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    check_gl_error("draw_rectangle");
}

void draw_circle(Vector2 position, float radius, Color color) {
    glUseProgram(simple_program);

    GLint projection_location = glGetUniformLocation(simple_program, "projection");
    if (projection_location == -1) {
        print("Simple program uniform 'projection' was not found\n");
    }
    else {
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, &world_projection._11);
    }

    GLint color_location = glGetUniformLocation(simple_program, "color");
    if (color_location == -1) {
        print("Simple program uniform 'color' was not found\n");
    }
    else {
        glUniform4f(color_location, color.r, color.g, color.b, color.a);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    Vector2 positions[360];

    for (int i = 0; i < count_of(positions); i++) {
        float angle = to_radians((float) i);
        positions[i] = position + (make_vector2(cosf(angle), sinf(angle)) * radius);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glDrawArrays(GL_LINE_LOOP, 0, count_of(positions));

    check_gl_error("draw_circle");
}

float get_sprite_draw_width(Sprite* sprite, float scale = 1.0f) {
    return sprite->width * PIXELS_TO_WORLD * scale;
}

float get_sprite_draw_height(Sprite* sprite, float scale = 1.0f) {
    return sprite->height * PIXELS_TO_WORLD * scale;
}

void draw_sprite(Sprite* sprite, Matrix4 transform) {
    glUseProgram(sprite_program);

    GLint projection_location = glGetUniformLocation(sprite_program, "projection");
    if (projection_location == -1) {
        print("Sprite program uniform 'projection' was not found\n");
    }
    else {
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, &world_projection._11);
    }

    GLint transform_location = glGetUniformLocation(sprite_program, "transform");
    if (transform_location == -1) {
        print("Sprite program uniform 'transform' was not found\n");
    }
    else {
        glUniformMatrix4fv(transform_location, 1, GL_FALSE, &transform._11);
    }

    glBindTexture(GL_TEXTURE_2D, sprite->texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    float width  = get_sprite_draw_width(sprite);
    float height = get_sprite_draw_height(sprite);

    float x = -width  / 2.0f;
    float y = -height / 2.0f;

    Vector2 positions[] = {
        make_vector2(x,         y),
        make_vector2(x + width, y),
        make_vector2(x,         y + height),
        make_vector2(x + width, y + height)
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    Vector2 uvs[] = {
        make_vector2(0.0f, 1.0f),
        make_vector2(1.0f, 1.0f),
        make_vector2(0.0f, 0.0f),
        make_vector2(1.0f, 0.0f)
    };

    glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(uvs), uvs);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) sizeof(positions));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    check_gl_error("draw_sprite");
}

void draw_sprite(Sprite* sprite, Vector2 position, float orientation = 0.0f, float scale = 1.0f) {
    draw_sprite(sprite, make_transform(position, orientation, scale));
}

float get_text_draw_width(char* text) {
    int text_length = get_string_length(text);

    float x_offset = 0.0f;
    float y_offset = 0.0f;

    for (int i = 0; i < text_length; i++) {
        char character = text[i];
        assert(character >= 32 && character < 32 + 96);

        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(font_glyphs, 512, 512, character - 32, &x_offset, &y_offset, &quad, 1);
    }

    return x_offset;
}

float get_text_draw_height(char* text) {
    return -1.0f;
}

void draw_text(char* text, Vector2 position, Color color = WHITE) {
    glUseProgram(font_program);

    GLint projection_location = glGetUniformLocation(font_program, "projection");
    if (projection_location == -1) {
        print("Font program uniform 'projection' was not found!\n");
    }
    else {
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, &gui_projection._11);
    }

    GLint color_location = glGetUniformLocation(font_program, "font_color");
    if (color_location == -1) {
        print("Font program uniform 'color' was not found!\n");
    }
    else {
        glUniform4f(color_location, color.r, color.g, color.b, color.a);
    }

    glBindTexture(GL_TEXTURE_2D, font_texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    
    float x_offset = 0.0f;
    float y_offset = 0.0f;

    int text_length = get_string_length(text);

    for (int i = 0; i < text_length; i++) {
        char character = text[i];
        assert(character >= 32 && character < 32 + 96);

        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(font_glyphs, 512, 512, character - 32, &x_offset, &y_offset, &quad, 1);

        Vector2 positions[] = {
            make_vector2(position.x + quad.x0, position.y - quad.y1),
            make_vector2(position.x + quad.x1, position.y - quad.y1),
            make_vector2(position.x + quad.x0, position.y - quad.y0),
            make_vector2(position.x + quad.x1, position.y - quad.y0)
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        Vector2 uvs[] = {
            make_vector2(quad.s0, quad.t1),
            make_vector2(quad.s1, quad.t1),
            make_vector2(quad.s0, quad.t0),
            make_vector2(quad.s1, quad.t0)
        };

        glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(uvs), uvs);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) sizeof(positions));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    check_gl_error("draw_text");
}