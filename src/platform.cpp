#define _CRT_SECURE_NO_WARNINGS 1

#if OS_WINDOWS
    #pragma warning(disable: 4100)
    #pragma warning(disable: 4189)
    #pragma warning(disable: 4456)
    #pragma warning(disable: 4800)

    #define WIN32_LEAN_AND_MEAN
    #define STRICT

    #include <windows.h>
    #include <windowsx.h>
    #include <gl/gl.h>
    #include <xinput.h>
    #include <xaudio2.h>

    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "gdi32.lib")
    #pragma comment(lib, "opengl32.lib")
    #pragma comment(lib, "xinput.lib")
    #pragma comment(lib, "xaudio2.lib")
#elif OS_LINUX
    #pragma GCC diagnostic ignored "-Wwrite-strings"
    #pragma GCC diagnostic ignored "-Wformat-security"

    #define Time __Time
    #define Font __Font
    
    #include <X11/Xlib.h>
    #include <GL/gl.h>
    #include <GL/glx.h>

    #undef Time
    #undef Font
#else
    #error "Unrecognized platform"
#endif

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
typedef u16      utf16;
typedef u32      utf32;

#define null 0

#define size_of(type)           ((u32) sizeof(type))
#define count_of(array)         (size_of(array) / size_of(array[0]))
#define offset_of(type, member) ((u32) ((type*) null)->member)

u32 to_u32(void* address) {
    return (u32)(u64) address;
}

const u32 TEMP_MEMORY_SIZE = 512 * 1024;

struct Platform {
    #if OS_WINDOWS
        HANDLE          process_heap;
        HWND            window;
        HDC             device_context;
        HGLRC           rendering_context;
        WINDOWPLACEMENT previous_window_placement = { size_of(previous_window_placement) };
    #elif OS_LINUX
        Display*   display;
        Window     window;
        GLXContext gl_context;
    #endif

    bool should_quit   = false;
    bool is_active     = false;
    bool is_fullscreen = false;

    u32 window_width  = 0;
    u32 window_height = 0;

    u32 heap_memory_allocated       = 0;
    u32 heap_memory_high_water_mark = 0;

    u8* temp_memory                 = null;
    u32 temp_memory_allocated       = 0;
    u32 temp_memory_high_water_mark = 0;
};

Platform platform;

// @note: Would like to do 'Time', but time() is a function in
// time.h. I couldn't find a way around it
struct Timers {
    #if OS_WINDOWS
        u64 frequency = 0;
        u64 start     = 0;
        u64 last      = 0;
        u64 current   = 0;
    #elif OS_LINUX
        timespec start;
        timespec last;
        timespec current;
    #endif

    f32 now   = 0.0f;
    f32 delta = 0.0f;
};

Timers timers;

#if OS_LINUX
    // @note: Taken from http://www.geonius.com/software/source/libgpl/ts_util.c
    timespec subtract_timespecs(timespec time1, timespec time2) {
        timespec result;

        if ((time1.tv_sec < time2.tv_sec) || ((time1.tv_sec == time2.tv_sec) && (time1.tv_nsec <= time2.tv_nsec))) {
            result.tv_sec = result.tv_nsec = 0;
        }
        else {
            result.tv_sec = time1.tv_sec - time2.tv_sec;
            if (time1.tv_nsec < time2.tv_nsec) {
                result.tv_nsec = time1.tv_nsec + 1000000000L - time2.tv_nsec;
                result.tv_sec -= 1;
            }
            else {
                result.tv_nsec = time1.tv_nsec - time2.tv_nsec;
            }
        }

        return result;
    }

    // @note: Taken from http://www.geonius.com/software/source/libgpl/ts_util.c
    f64 timespec_to_f64(timespec time) {
        return (f64) time.tv_sec + (time.tv_nsec / 1000000000.0);
    }
#endif

struct Key {
    bool up   = false;
    bool down = false;
    bool held = false;
};

void update_key(Key* key, bool is_down) {
    key->up   = false;
    key->down = false;

    if (is_down) {
        if (!key->held) {
            key->down = true;
            key->held = true;
        }
    }
    else {
        if (key->held) {
            key->up   = true;
            key->held = false;
        }
    }
}

struct Input {
    Key mouse_left;
    Key mouse_right;
    Key key_escape;
    Key key_space;

    Key key_w;
    Key key_a;
    Key key_s;
    Key key_d;

    i32 mouse_x = 0;
    i32 mouse_y = 0;

    Key gamepad_start;
    Key gamepad_a;
    Key gamepad_b;
    Key gamepad_x;
    Key gamepad_y;

    Key gamepad_left_trigger;
    Key gamepad_right_trigger;

    f32 gamepad_left_x;
    f32 gamepad_left_y;
    f32 gamepad_right_x;
    f32 gamepad_right_y;
};

Input input;

#if DEBUG
    #define assert(expression)                                              \
        do {                                                                \
            if (!(expression)) __assert(__FILE__, __LINE__, #expression);   \
        } while (0)
#else
    #define assert(expression)
#endif

#define invalid_default_case() default: assert(!"Invalid default case"); break

u32 get_length(utf8* string) {
    utf8* cursor = string;
    while (*cursor++);

    return (u32) (cursor - string);
}

bool compare(utf8* string_a, utf8* string_b) {
    u32 a_length = get_length(string_a);
    u32 b_length = get_length(string_b);

    if (a_length != b_length) return false;

    for (u32 i = 0; i < a_length; i++) {
        if (string_a[i] != string_b[i]) return false;
    }

    return true;
}

u32 hash(utf8* string) {
    u32 length = get_length(string);

    u32 hash = 5381;
    for (u32 i = 0; i < length; i++) {
        hash = ((hash << 5) + hash) + string[i];
    }

    return hash;
}

utf8* format_string_args(utf8* string, va_list args) {
    void* temp_alloc(u32 size);

    u32 size = vsnprintf(null, 0, string, args);

    utf8* result = (utf8*) temp_alloc(size + 1);
    vsprintf(result, string, args);

    return result;
}

utf8* format_string(utf8* string, ...) {
    va_list args;
    va_start(args, string);

    utf8* result = format_string_args(string, args);
    va_end(args);

    return result;
}

void __assert(utf8* file_name, u32 line_number, utf8* expression_string) {
    utf8* message = format_string("Assertion failed at %s (%u), %s\n", file_name, line_number, expression_string);
    printf(message);

    #if OS_WINDOWS
        if (IsDebuggerPresent()) {
            DebugBreak();
        }

        ExitProcess(1);
    #else
        exit(EXIT_FAILURE);
    #endif
}

void* heap_alloc(u32 size) {
    #if OS_WINDOWS
        platform.heap_memory_allocated += size;
        
        if (platform.heap_memory_allocated > platform.heap_memory_high_water_mark) {
            platform.heap_memory_high_water_mark = platform.heap_memory_allocated;
        }

        void* result = HeapAlloc(platform.process_heap, 0, size);
    #elif OS_LINUX
        // @todo: Implement using mmap?
        void* result = malloc(size);
    #endif

    assert(result);

    return result;
}

void heap_dealloc(void* memory) {
    #if OS_WINDOWS
        if (!memory) return;
        assert(HeapValidate(platform.process_heap, 0, memory));

        u32 size = (u32) HeapSize(platform.process_heap, 0, memory);
        platform.heap_memory_allocated -= size;

        HeapFree(platform.process_heap, 0, memory);
    #elif OS_LINUX
        free(memory);
    #endif
}

void* heap_realloc(void* memory, u32 new_size) {
    #if OS_WINDOWS
        if (!memory) return heap_alloc(new_size);
        assert(HeapValidate(platform.process_heap, 0, memory));

        u32 old_size = (u32) HeapSize(platform.process_heap, 0, memory);
        
        platform.heap_memory_allocated -= old_size;
        platform.heap_memory_allocated += new_size;

        void* result = HeapReAlloc(platform.process_heap, 0, memory, new_size);
    #elif OS_LINUX
        void* result = realloc(memory, new_size);
    #endif

    assert(result);
    
    return result;
}

void* temp_alloc(u32 size) {
    // @todo: Remove these from temp_alloc, this would be a recursive call because
    // assert calls temp_alloc
    // Need to hardcode something for temp alloc that is special, I think
    
    assert(platform.temp_memory);
    assert(platform.temp_memory_allocated + size <= TEMP_MEMORY_SIZE);

    void* result = platform.temp_memory + platform.temp_memory_allocated;
    platform.temp_memory_allocated += size;

    if (platform.temp_memory_allocated > platform.temp_memory_high_water_mark) {
        platform.temp_memory_high_water_mark = platform.temp_memory_allocated;
    }

    return result;
}

void temp_dealloc(void* memory) {
    
}

// @todo: Implement natively
void* read_entire_file(utf8* file_name) {
    void* result = null;

    FILE* file = fopen(file_name, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);

        u32 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = (u8*) heap_alloc(file_size);
        fread(result, 1, file_size, file);

        fclose(file);
    }

    return result;
}

utf8* get_executable_directory() {
    utf8 buffer[MAX_PATH];
    u32 length = GetModuleFileName(GetModuleHandle(null), buffer, count_of(buffer));

    utf8* cursor = &buffer[length];
    while (cursor > buffer) {
        utf32 codepoint = *cursor;
        *cursor = null;

        if (codepoint == '\\') break;
        cursor -= 1;
    }

    utf8* result = (utf8*) temp_alloc(length + 1);
    for (u32 i = 0; i < length; i++) {
        result[i] = buffer[i];
    }

    result[length] = null;
    return result;
}

void toggle_fullscreen() {
    DWORD style = GetWindowLong(platform.window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        GetWindowPlacement(platform.window, &platform.previous_window_placement);

        MONITORINFO monitor_info = { size_of(monitor_info) };
        GetMonitorInfo(MonitorFromWindow(platform.window, MONITOR_DEFAULTTOPRIMARY), &monitor_info);

        SetWindowLong(platform.window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

        SetWindowPos(
            platform.window, 
            HWND_TOP, 
            monitor_info.rcMonitor.left, 
            monitor_info.rcMonitor.top, 
            monitor_info.rcMonitor.right  - monitor_info.rcMonitor.left, 
            monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top, 
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        platform.is_fullscreen = true;
    }
    else {
        SetWindowLong(platform.window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(platform.window, &platform.previous_window_placement);

        SetWindowPos(
            platform.window, 
            null, 
            0, 
            0, 
            0, 
            0, 
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        platform.is_fullscreen = false;
    }
}

#if OS_WINDOWS
    LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
        LRESULT result = 0;

        switch (message) {
            case WM_DESTROY: {
                platform.should_quit = true;
                break;
            }
            case WM_ACTIVATE: {
                switch (LOWORD(w_param)) {
                    case WA_CLICKACTIVE:
                    case WA_ACTIVE: {
                        platform.is_active = true;
                        break;
                    }
                    case WA_INACTIVE: {
                        platform.is_active = false;
                        break;
                    }
                }

                break;
            }
            case WM_SIZE: {
                platform.window_width  = LOWORD(l_param);
                platform.window_height = HIWORD(l_param);

                break;
            }
            default: {
                result = DefWindowProc(window, message, w_param, l_param);
                break;
            }
        }

        return result;
    }
#endif

void init_platform() {
    #if OS_WINDOWS
        platform.process_heap = GetProcessHeap();
    #endif

    platform.temp_memory = (u8*) heap_alloc(TEMP_MEMORY_SIZE);

    #if OS_WINDOWS
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        timers.frequency = frequency.QuadPart;

        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);

        timers.start   = start.QuadPart;
        timers.last    = start.QuadPart;
        timers.current = start.QuadPart;

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

        u32 screen_width  = GetSystemMetrics(SM_CXSCREEN);
        u32 screen_height = GetSystemMetrics(SM_CYSCREEN);

        platform.window_width  = 1600;
        platform.window_height = 900;

        platform.window = CreateWindow(
            window_class.lpszClassName, 
            "Asteroids!", 
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
            (screen_width  / 2) - (platform.window_width  / 2), 
            (screen_height / 2) - (platform.window_height / 2), 
            platform.window_width, 
            platform.window_height, 
            null, 
            null, 
            window_class.hInstance, 
            null);

        platform.device_context = GetDC(platform.window);

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

        i32 pfd_id = ChoosePixelFormat(platform.device_context, &pfd);
        SetPixelFormat(platform.device_context, pfd_id, &pfd);

        platform.rendering_context = wglCreateContext(platform.device_context);
        wglMakeCurrent(platform.device_context, platform.rendering_context);
    #elif OS_LINUX
        timespec now;
        clock_gettime(CLOCK_REALTIME, &now);

        timers.start   = now;
        timers.last    = now;
        timers.current = now;

        platform.display = XOpenDisplay(null);
        assert(platform.display);

        Window root = DefaultRootWindow(platform.display);
        
        i32 gl_attributes[] = {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            None
        };

        XVisualInfo* visual_info = glXChooseVisual(platform.display, 0, gl_attributes);
        assert(visual_info);

        Colormap color_map = XCreateColormap(platform.display, root, visual_info->visual, AllocNone);

        XSetWindowAttributes window_attributes;

        window_attributes.colormap   = color_map;
        window_attributes.event_mask = ResizeRedirectMask;

        platform.window_width  = 600;
        platform.window_height = 600;

        platform.window = XCreateWindow(
            platform.display, 
            root, 
            0, 
            0, 
            platform.window_width, 
            platform.window_height, 
            0,
            visual_info->depth, 
            InputOutput, 
            visual_info->visual, 
            CWColormap | CWEventMask, 
            &window_attributes);

        XStoreName(platform.display, platform.window, "Asteroids!");
        XMapWindow(platform.display, platform.window);

        platform.gl_context = glXCreateContext(platform.display, visual_info, null, GL_TRUE);
        glXMakeCurrent(platform.display, platform.window, platform.gl_context);
    #endif
}

#if OS_WINDOWS
    f32 process_xinput_stick(i16 value, i16 dead_zone) {
        f32 result = 0.0f;

        if (value < -dead_zone) {
            result = (f32) (value + dead_zone) / (f32) (-INT16_MIN - dead_zone);
        }
        else if (value > dead_zone) {
            result = (f32) (value - dead_zone) / (f32) (INT16_MAX - dead_zone);
        }

        return result;
    }
#endif

void update_platform() {
    platform.temp_memory_allocated = 0;

    #if OS_WINDOWS
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);

        timers.last    = timers.current;
        timers.current = ticks.QuadPart;
        
        timers.now   = (f32) (timers.current - timers.start) / (f32) timers.frequency;
        timers.delta = (f32) (timers.current - timers.last)  / (f32) timers.frequency;

        MSG message;
        while (PeekMessage(&message, null, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        if (platform.is_active) {
            update_key(&input.mouse_left,  GetAsyncKeyState(VK_LBUTTON));
            update_key(&input.mouse_right, GetAsyncKeyState(VK_RBUTTON));
            update_key(&input.key_escape,  GetAsyncKeyState(VK_ESCAPE));
            update_key(&input.key_space,   GetAsyncKeyState(VK_SPACE));

            update_key(&input.key_w, GetAsyncKeyState('W'));
            update_key(&input.key_a, GetAsyncKeyState('A'));
            update_key(&input.key_s, GetAsyncKeyState('S'));
            update_key(&input.key_d, GetAsyncKeyState('D'));

            POINT cursor_position;
            
            GetCursorPos(&cursor_position);
            ScreenToClient(platform.window, &cursor_position);

            input.mouse_x = cursor_position.x;
            input.mouse_y = cursor_position.y;

            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                XINPUT_STATE state;
                if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                    XINPUT_GAMEPAD* gamepad = &state.Gamepad;

                    update_key(&input.gamepad_start, (gamepad->wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START);
                    update_key(&input.gamepad_a,     (gamepad->wButtons & XINPUT_GAMEPAD_A)     == XINPUT_GAMEPAD_A);
                    update_key(&input.gamepad_b,     (gamepad->wButtons & XINPUT_GAMEPAD_B)     == XINPUT_GAMEPAD_B);
                    update_key(&input.gamepad_x,     (gamepad->wButtons & XINPUT_GAMEPAD_X)     == XINPUT_GAMEPAD_X);
                    update_key(&input.gamepad_y,     (gamepad->wButtons & XINPUT_GAMEPAD_Y)     == XINPUT_GAMEPAD_Y);

                    update_key(&input.gamepad_left_trigger,  gamepad->bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
                    update_key(&input.gamepad_right_trigger, gamepad->bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

                    input.gamepad_left_x  = process_xinput_stick(gamepad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    input.gamepad_left_y  = process_xinput_stick(gamepad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    input.gamepad_right_x = process_xinput_stick(gamepad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                    input.gamepad_right_y = process_xinput_stick(gamepad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                }
            }
        }
    #elif OS_LINUX
        timespec now;
        clock_gettime(CLOCK_REALTIME, &now);

        timers.last    = timers.current;
        timers.current = now;
        
        timers.now   = (f32) (timespec_to_f64(subtract_timespecs(timers.current, timers.start)));
        timers.delta = (f32) (timespec_to_f64(subtract_timespecs(timers.current, timers.last)));

        while (XPending(platform.display)) {
            XEvent event;
            XNextEvent(platform.display, &event);

            switch(event.type) {
                case ResizeRedirectMask: {
                    platform.window_width  = event.xresizerequest.width;
                    platform.window_height = event.xresizerequest.height;

                    break;
                }
                invalid_default_case();
            }
        }
    #endif

    if (timers.delta > 0.1f) timers.delta = 0.1f;
}

void swap_buffers() {
    #if OS_WINDOWS
        SwapBuffers(platform.device_context);
    #elif OS_LINUX
        glXSwapBuffers(platform.display, platform.window);
    #endif
}