#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <windowsx.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <stdint.h>

// @todo: drop assert, stdlib (random), and stdio (file/console io)
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;
typedef char     utf8;
typedef char16_t utf16;
typedef char32_t utf32;

#define null 0

#define size_of(type)           ((u32) sizeof(type))
#define count_of(array)         (size_of(array) / size_of(array[0]))
#define offset_of(type, member) ((u32) ((type*) null)->member)

#define invalid_default_case() assert(!"Invalid default case")

typedef void* Alloc(u32 size);
typedef void  Dealloc(void* memory);

struct Allocator {
    Alloc*   alloc   = null;
    Dealloc* dealloc = null;
};

Allocator make_allocator(Alloc* alloc, Dealloc* dealloc) {
    Allocator allocator;

    allocator.alloc   = alloc;
    allocator.dealloc = dealloc;

    return allocator;
}

Allocator* allocators[32];
u32 allocators_index = 0;

Allocator* default_allocator = null;

Allocator* get_current_allocator() {
    Allocator* allocator = null;
    if (allocators_index) {
        allocator = allocators[allocators_index];
    }
    else {
        allocator = default_allocator;
    }

    assert(allocator);

    assert(allocator->alloc);
    assert(allocator->dealloc);

    return allocator;
}

void push_allocator(Allocator* allocator) {
    if (!allocator) allocator = get_current_allocator();

    allocators_index += 1;
    allocators[allocators_index] = allocator;
}

void pop_allocator() {
    allocators[allocators_index] = null;
    allocators_index -= 1;
}

void* operator new(size_t size)               { return get_current_allocator()->alloc(size); }
void* operator new(size_t size, void* memory) { return memory; }
void  operator delete(void* memory)           { get_current_allocator()->dealloc(memory); }

Allocator heap_allocator;

HANDLE process_heap;

u32 heap_memory_allocated;
u32 heap_memory_high_water_mark;

void* heap_alloc(u32 size) {
    heap_memory_allocated += size;
    
    if (heap_memory_allocated > heap_memory_high_water_mark) {
        heap_memory_high_water_mark = heap_memory_allocated;
    }

    return HeapAlloc(process_heap, 0, size);
}

void heap_dealloc(void* memory) {
    u32 size = HeapSize(process_heap, 0, memory);
    heap_memory_allocated -= size;

    HeapFree(process_heap, 0, memory);
}

Allocator temp_allocator;

const u32 TEMP_MEMORY_SIZE = 32 * 1024;

u8* temp_memory;
u32 temp_memory_allocated;
u32 temp_memory_high_water_mark;

void* temp_alloc(u32 size) {
    assert(temp_memory_allocated + size <= TEMP_MEMORY_SIZE);

    void* result = temp_memory + temp_memory_allocated;
    temp_memory_allocated += size;

    if (temp_memory_allocated > temp_memory_high_water_mark) {
        temp_memory_high_water_mark = temp_memory_allocated;
    }

    return result;
}

void temp_dealloc(void* memory) {

}

#include "data_structures.cpp"
#include "math.cpp"

void seed_random(u32 seed) {
    srand(seed);
}

u32 get_random_u32() {
    return rand();
}

u32 get_random_out_of(u32 count) {
    return get_random_u32() % count;
}

bool get_random_chance(u32 chance) {
    return get_random_out_of(chance) == chance - 1;
}

f32 get_random_unilateral() {
    return (f32) get_random_u32() / (f32) RAND_MAX;
}

f32 get_random_bilateral() {
    return (2.0f * get_random_unilateral()) - 1.0f;
}

f32 get_random_between(f32 min, f32 max) {
    return lerp(min, get_random_unilateral(), max);
}

i32 get_random_between(i32 min, i32 max) {
    return min + ((i32) get_random_u32() % ((max + 1) - min));
}

utf8* format_string_args(utf8* string, va_list args) {
    u32 size = vsnprintf(null, 0, string, args);

    utf8* result = (utf8*) temp_alloc(size + 1);
    vsprintf(result, string, args);

    return result;
}

utf8* format_string(utf8* string, ...) {
    va_list args;
    va_start(args, string);

    string = format_string_args(string, args);
    va_end(args);

    return string;
}

void* read_entire_file(utf8* file_name) {
    void* result = null;

    FILE* file = fopen(file_name, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);

        u32 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = heap_alloc(file_size);
        fread(result, 1, file_size, file);

        fclose(file);
    }

    return result;
}

const u32 WINDOW_WIDTH  = 1600;
const u32 WINDOW_HEIGHT = 900;

struct Time {
    u64 ticks_frequency = 0;
    u64 ticks_start     = 0;
    u64 ticks_last      = 0;
    u64 ticks_current   = 0;
    
    f32 now   = 0.0f;
    f32 delta = 0.0f;
};

HWND window;
Time time;

bool should_quit        = false;
u32  highest_water_mark = 0;

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

i32 main() {
    heap_allocator = make_allocator(heap_alloc, heap_dealloc);
    temp_allocator = make_allocator(temp_alloc, temp_dealloc);

    default_allocator = &heap_allocator;

    process_heap = GetProcessHeap();
    temp_memory  = (u8*) heap_alloc(TEMP_MEMORY_SIZE);

    WNDCLASS window_class = {
        CS_OWNDC,
        window_proc,
        0,
        0,
        GetModuleHandle(null),
        null,
        LoadCursor(null, IDC_ARROW),
        (HBRUSH) GetStockObject(BLACK_BRUSH),
        null,
        "WINDOW_CLASS"
    };

    RegisterClass(&window_class);

    u32 window_x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (WINDOW_WIDTH  / 2);
    u32 window_y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (WINDOW_HEIGHT / 2);

    window = CreateWindow(
        window_class.lpszClassName, 
        "Asteroids!", 
        WS_POPUP | WS_VISIBLE, 
        window_x, 
        window_y, 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        null, 
        null, 
        window_class.hInstance, 
        null);

    HDC device_context = GetDC(window);

    PIXELFORMATDESCRIPTOR pfd = {
        size_of(PIXELFORMATDESCRIPTOR),
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

    i32 pfd_id = ChoosePixelFormat(device_context, &pfd);
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
        temp_memory_allocated = 0;

        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);

        time.ticks_last    = time.ticks_current;
        time.ticks_current = ticks.QuadPart;

        time.now   = (f32) (time.ticks_current - time.ticks_start) / (f32) time.ticks_frequency;
        time.delta = (f32) (time.ticks_current - time.ticks_last)  / (f32) time.ticks_frequency;

        MSG message;
        while (PeekMessage(&message, null, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        update_input();
        update_sound();
        
        update_game();

        SwapBuffers(device_context);
    }
    
    return 0;
}