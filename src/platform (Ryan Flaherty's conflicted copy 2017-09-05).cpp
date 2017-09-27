#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

const int SCREEN_WIDTH       = 1600;
const int SCREEN_HEIGHT      = 900;

const int HALF_SCREEN_WIDTH  = SCREEN_WIDTH  / 2;
const int HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

const HANDLE PROCESS_HEAP = GetProcessHeap();

void* alloc(int size) {
    return HeapAlloc(PROCESS_HEAP, 0, size);
}

void dealloc(void* memory) {
    HeapFree(PROCESS_HEAP, 0, memory);
}

const HANDLE STD_OUT = GetStdHandle(STD_OUTPUT_HANDLE);
char string_buffer[4066];

char* format_string(char* format, ...) {
    va_list args;
    va_start(args, format);

    StringCchVPrintf(string_buffer, count_of(string_buffer), format, args);
    va_end(args);

    return string_buffer;
}

void print(char* message, ...) {
    va_list args;
    va_start(args, message);

    StringCchVPrintf(string_buffer, count_of(string_buffer), message, args);
    WriteConsole(STD_OUT, string_buffer, get_string_length(string_buffer), NULL, NULL);

    va_end(args);
}

#define assert(expression) _assert(expression, __FILE__, __LINE__, #expression)

void _assert(bool expression, char* file_name, int line_number, char* expression_string) {
    if (!(expression)) {
        print("%s (%i): Assertion failed for expression '%s'\n", file_name, line_number, expression_string);
        ExitProcess(1);
    }
}

void* read_entire_file(char* file_name) {
    void* result = NULL;

    HANDLE file_handle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD file_size = GetFileSize(file_handle, NULL);
        result = alloc(file_size);

        DWORD bytes_read;
        ReadFile(file_handle, result, file_size, &bytes_read, NULL);

        CloseHandle(file_handle);
    }
    else {
        print("Failed to read file '%s'\n", file_name);
    }

    return result;
}

struct Key {
    bool up;
    bool down;
    bool held;
};

struct Input {
    float mouse_x;
    float mouse_y;

    Key any_key;
    Key left_mb;
    Key escape;
    Key space;
};

void reset_key(Key* key) {
    key->up   = false;
    key->down = false;
}

void set_key_up(Key* key) {
    if (!key->held) return;

    key->up   = true;
    key->held = false;
}

void set_key_down(Key* key) {
    if (key->held) return;

    key->held = true;
    key->down = true;
}

bool  is_running = true;
Input input;

RECT old_clip_rect;

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    switch (message) {
        case WM_ACTIVATE: {
            if (LOWORD(w_param) != WA_INACTIVE) {
                GetClipCursor(&old_clip_rect);

                RECT client_rect;
                GetWindowRect(window, &client_rect);

                ClipCursor(&client_rect);
            }
            else {
                ClipCursor(&old_clip_rect);
            }

            break;
        }
        case WM_CLOSE: {
            is_running = false;
            break;
        }
        case WM_MOUSEMOVE: {
            input.mouse_x = GET_X_LPARAM(l_param);
            input.mouse_y = GET_Y_LPARAM(l_param);

            break;
        }
        case WM_LBUTTONUP: {
            set_key_up(&input.left_mb);
            break;
        }
        case WM_LBUTTONDOWN: {
            set_key_down(&input.left_mb);
            break;
        }
        case WM_KEYUP: {
            set_key_up(&input.any_key);

            switch (w_param) {
                case VK_ESCAPE: {
                    set_key_up(&input.escape);
                    break;
                }
                case VK_SPACE: {
                    set_key_up(&input.space);
                    break;
                }
                default: {
                    result = DefWindowProc(window, message, w_param, l_param);
                    break;
                }
            }

            break;
        }
        case WM_KEYDOWN: {
            set_key_down(&input.any_key);

            switch (w_param) {
                case VK_ESCAPE: {
                    set_key_down(&input.escape);
                    break;
                }
                case VK_SPACE: {
                    set_key_down(&input.space);
                    break;
                }
                default: {
                    result = DefWindowProc(window, message, w_param, l_param);
                    break;
                }
            }

            break;
        }
        default: {
            result = DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return result;
}

HWND window;

uint64_t tick_frequency;
uint64_t last_ticks;
uint64_t start_ticks;

float delta_time;

uint64_t get_ticks() {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return counter.QuadPart;
}

void init_platform() {
    WNDCLASS window_class = {};
    window_class.style         = CS_OWNDC;
    window_class.hInstance     = GetModuleHandle(NULL);
    window_class.lpfnWndProc   = window_proc;
    window_class.lpszClassName = "WINDOW_CLASS";
    window_class.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&window_class);

    window = CreateWindow(
        window_class.lpszClassName, 
        "Asteroids!", 
        WS_POPUPWINDOW, 
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (SCREEN_WIDTH  / 2), 
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (SCREEN_HEIGHT / 2), 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        NULL, 
        NULL, 
        window_class.hInstance, 
        NULL);

    ShowWindow(window, TRUE);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    tick_frequency = frequency.QuadPart;
}

void update_platform() {
    start_ticks = get_ticks();
    if (last_ticks == 0) last_ticks = start_ticks;
    
    delta_time = (double) (start_ticks - last_ticks) / (double) tick_frequency;
    last_ticks = start_ticks;

    if (delta_time > 1.0f) delta_time = 1.0f;

    reset_key(&input.any_key);
    reset_key(&input.left_mb);
    reset_key(&input.escape);
    reset_key(&input.space);

    MSG window_message;
    while (PeekMessage(&window_message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&window_message);
        DispatchMessage(&window_message);
    }
}