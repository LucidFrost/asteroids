#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define count_of(array) (sizeof(array) / sizeof(array[0]))

void* operator new(size_t count)            { return malloc(count); }
void* operator new(size_t count, void* ptr) { return ptr; }
void  operator delete(void* ptr)            { free(ptr); }

// @todo: temporary storage
char* format_string_args(char* string, va_list args) {
    static char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), string, args);

    return buffer;
}

char* format_string(char* string, ...) {
    va_list args;
    va_start(args, string);

    string = format_string_args(string, args);
    va_end(args);

    return string;
}

void* read_entire_file(char* file_name) {
    void* result = NULL;

    FILE* file = fopen(file_name, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);

        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = malloc(file_size);
        fread(result, 1, file_size, file);

        fclose(file);
    }

    return result;
}

const int WINDOW_WIDTH  = 1600;
const int WINDOW_HEIGHT = 900;

struct Key {
    bool up   = false;
    bool down = false;
    bool held = false;
};

void on_key_update(Key* key) {
    key->up   = false;
    key->down = false;
}

void on_key_down(Key* key) {
    if (key->held) return;

    key->down = true;
    key->held = true;
}

void on_key_up(Key* key) {
    if (!key->held) return;
    
    key->up   = true;
    key->held = false;
}

struct Input {
    Key key_escape;

    Key key_w;
    Key key_a;
    Key key_s;
    Key key_d;
};

Input input;

struct Time {
    uint64_t ticks_frequency = 0;
    uint64_t ticks_start     = 0;
    uint64_t ticks_last      = 0;
    uint64_t ticks_current   = 0;
    
    float now   = 0.0f;
    float delta = 0.0f;
};

Time time;

#include "math.cpp"
#include "draw.cpp"
#include "asteroids.cpp"

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    switch (message) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_KEYUP: {
            #define key_up_case(key, member)    \
                case key: {                     \
                    on_key_up(&input.member);   \
                    break;                      \
                }

            switch (w_param) {
                key_up_case(VK_ESCAPE, key_escape);

                key_up_case('W', key_w);
                key_up_case('A', key_a);
                key_up_case('S', key_s);
                key_up_case('D', key_d);

                default: {
                    result = DefWindowProc(window, message, w_param, l_param);
                    break;
                }
            }

            #undef key_up_case

            break;
        }
        case WM_KEYDOWN: {
            #define key_down_case(key, member)  \
                case key: {                     \
                    on_key_down(&input.member); \
                    break;                      \
                }

            switch (w_param) {
                key_down_case(VK_ESCAPE, key_escape);
                
                key_down_case('W', key_w);
                key_down_case('A', key_a);
                key_down_case('S', key_s);
                key_down_case('D', key_d);

                default: {
                    result = DefWindowProc(window, message, w_param, l_param);
                    break;
                }
            }

            #undef key_down_case

            break;
        }
        default: {
            result = DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return result;
}

int main() {
    WNDCLASS window_class = {
        CS_OWNDC,
        window_proc,
        0,
        0,
        GetModuleHandle(NULL),
        NULL,
        NULL,
        (HBRUSH) GetStockObject(BLACK_BRUSH),
        NULL,
        "WINDOW_CLASS"
    };

    RegisterClass(&window_class);

    int window_x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (WINDOW_WIDTH  / 2);
    int window_y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (WINDOW_HEIGHT / 2);

    HWND window = CreateWindow(
        window_class.lpszClassName, 
        "Asteroids!", 
        WS_POPUP | WS_VISIBLE, 
        window_x, 
        window_y, 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        NULL, 
        NULL, 
        window_class.hInstance, 
        NULL);

    HDC device_context = GetDC(window);

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pfd_id = ChoosePixelFormat(device_context, &pfd);
    SetPixelFormat(device_context, pfd_id, &pfd);

    HGLRC rendering_context = wglCreateContext(device_context);
    wglMakeCurrent(device_context, rendering_context);

    LARGE_INTEGER ticks_frequency;
    QueryPerformanceFrequency(&ticks_frequency);

    LARGE_INTEGER ticks_start;
    QueryPerformanceCounter(&ticks_start);

    time.ticks_frequency = ticks_frequency.QuadPart;
    time.ticks_start     = ticks_start.QuadPart;
    time.ticks_last      = ticks_start.QuadPart;
    time.ticks_current   = ticks_start.QuadPart;

    init_draw();
    init_asteroids();

    while (1) {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);

        time.ticks_last    = time.ticks_current;
        time.ticks_current = ticks.QuadPart;

        time.now   = (float) (time.ticks_current - time.ticks_start) / (float) time.ticks_frequency;
        time.delta = (float) (time.ticks_current - time.ticks_last)  / (float) time.ticks_frequency;

        on_key_update(&input.key_escape);

        on_key_update(&input.key_w);
        on_key_update(&input.key_a);
        on_key_update(&input.key_s);
        on_key_update(&input.key_d);

        bool should_quit = false;

        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                should_quit = true;
            }
            else {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }

        if (should_quit)           break;
        if (input.key_escape.down) break;

        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        update_asteroids();

        SwapBuffers(device_context);
    }
    
    return 0;
}