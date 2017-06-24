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

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef  int8_t  i8;
typedef  int16_t i16;
typedef  int32_t i32;
typedef  int64_t i64;

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

bool get_random_chance(int chance) {
    return get_random_out_of(chance) == chance - 1;
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

HWND window;
bool should_quit;

struct Time {
    uint64_t ticks_frequency = 0;
    uint64_t ticks_start     = 0;
    uint64_t ticks_last      = 0;
    uint64_t ticks_current   = 0;
    
    float now   = 0.0f;
    float delta = 0.0f;
};

Time time;

#include "input.cpp"
#include "draw.cpp"
#include "sound.cpp"

#include "game.cpp"

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    switch (message) {
        case WM_DESTROY: {
            should_quit = true;
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

    window = CreateWindow(
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
    init_sound();
    
    init_game();

    while (!should_quit) {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);

        time.ticks_last    = time.ticks_current;
        time.ticks_current = ticks.QuadPart;

        time.now   = (float) (time.ticks_current - time.ticks_start) / (float) time.ticks_frequency;
        time.delta = (float) (time.ticks_current - time.ticks_last)  / (float) time.ticks_frequency;

        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        update_input();
        update_game();

        SwapBuffers(device_context);
    }
    
    return 0;
}