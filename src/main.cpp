#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <windowsx.h>

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

#include "math.cpp"

void seed_random(int seed) {
    srand(seed);
}

int get_random_int() {
    return rand();
}

int get_random_out_of(int count) {
    return get_random_int() % count;
}

float get_random_unilateral() {
    return (float) get_random_int() / (float) RAND_MAX;
}

float get_random_bilateral() {
    return (2.0f * get_random_unilateral()) - 1.0f;
}

float get_random_between(float min, float max) {
    return lerp(min, get_random_unilateral(), max);
}

int get_random_between(int min, int max) {
    return min + (get_random_int() % ((max + 1) - min));
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
    int mouse_x;
    int mouse_y;

    Key mouse_left;
    Key mouse_right;

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

#include "draw.cpp"
#include "game.cpp"

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    switch (message) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_MOUSEMOVE: {
            input.mouse_x = GET_X_LPARAM(l_param);
            input.mouse_y = GET_Y_LPARAM(l_param);

            break;
        }
        case WM_LBUTTONUP: {
            on_key_up(&input.mouse_left);
            break;
        }
        case WM_LBUTTONDOWN: {
            on_key_down(&input.mouse_left);
            break;
        }
        case WM_RBUTTONUP: {
            on_key_up(&input.mouse_right);
            break;
        }
        case WM_RBUTTONDOWN: {
            on_key_down(&input.mouse_right);
            break;
        }
        case WM_KEYUP: {
            #define case(key, member)    \
                case key: {                     \
                    on_key_up(&input.member);   \
                    break;                      \
                }

            switch (w_param) {
                case(VK_ESCAPE, key_escape);

                case('W', key_w);
                case('A', key_a);
                case('S', key_s);
                case('D', key_d);

                default: {
                    result = DefWindowProc(window, message, w_param, l_param);
                    break;
                }
            }

            #undef case

            break;
        }
        case WM_KEYDOWN: {
            #define case(key, member)  \
                case key: {                     \
                    on_key_down(&input.member); \
                    break;                      \
                }

            switch (w_param) {
                case(VK_ESCAPE, key_escape);
                
                case('W', key_w);
                case('A', key_a);
                case('S', key_s);
                case('D', key_d);

                default: {
                    result = DefWindowProc(window, message, w_param, l_param);
                    break;
                }
            }

            #undef case

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
        LoadCursor(NULL, IDC_ARROW),
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

    seed_random(time.ticks_start);

    init_draw();
    init_game();

    while (1) {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);

        time.ticks_last    = time.ticks_current;
        time.ticks_current = ticks.QuadPart;

        time.now   = (float) (time.ticks_current - time.ticks_start) / (float) time.ticks_frequency;
        time.delta = (float) (time.ticks_current - time.ticks_last)  / (float) time.ticks_frequency;

        on_key_update(&input.mouse_left);
        on_key_update(&input.mouse_right);

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

        update_game();

        SwapBuffers(device_context);
    }
    
    return 0;
}