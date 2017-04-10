#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

#include <stdint.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define count_of(array) (sizeof(array) / sizeof(array[0]))

const HANDLE STD_OUT      = GetStdHandle(STD_OUTPUT_HANDLE);
const HANDLE PROCESS_HEAP = GetProcessHeap();

const int FORMAT_STRING_BUFFER_SIZE = 4096;
char format_string_buffer[FORMAT_STRING_BUFFER_SIZE];

const int WINDOW_WIDTH  = 1600;
const int WINDOW_HEIGHT = 900;

const int HALF_WINDOW_WIDTH  = WINDOW_WIDTH  / 2;
const int HALF_WINDOW_HEIGHT = WINDOW_HEIGHT / 2;

const float WORLD_HEIGHT = 15.0f;
const float WORLD_WIDTH  = WORLD_HEIGHT * ((float) WINDOW_WIDTH / (float) WINDOW_HEIGHT);

const float HALF_WORLD_WIDTH  = WORLD_WIDTH  / 2.0f;
const float HALF_WORLD_HEIGHT = WORLD_HEIGHT / 2.0f;

bool is_running  = true;
float delta_time = 0.0f;

void* alloc(int size) {
    return HeapAlloc(PROCESS_HEAP, 0, size);
}

void dealloc(void* memory) {
    HeapFree(PROCESS_HEAP, 0, memory);
}

int get_string_length(char* string) {
    char* at = string;
    while (*at++);

    return (int) (at - string - 1);
}

char* format_string(char* format, ...) {
    va_list args;
    va_start(args, format);

    StringCchVPrintf(format_string_buffer, FORMAT_STRING_BUFFER_SIZE, format, args);
    va_end(args);

    return format_string_buffer;
}

void print(char* message, ...) {
    va_list args;
    va_start(args, message);

    StringCchVPrintf(format_string_buffer, FORMAT_STRING_BUFFER_SIZE, message, args);
    WriteConsole(STD_OUT, format_string_buffer, get_string_length(format_string_buffer), NULL, NULL);

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
        print("Failed to open file '%s'\n", file_name);
    }

    return result;
}

#include "math.cpp"
#include "random.cpp"
#include "draw.cpp"

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

Input input;

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

enum Game_Mode {
    GM_NONE,
    GM_MENU,
    GM_GAME
};

Game_Mode game_mode = GM_NONE;
bool game_mode_switched = false;

void switch_game_mode(Game_Mode new_game_mode) {
    game_mode = new_game_mode;
    game_mode_switched = true;
}

#include "gm_menu.cpp"
#include "gm_game.cpp"

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    switch (message) {
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
            set_key_up(&input.any_key);
            set_key_up(&input.left_mb);

            break;
        }
        case WM_LBUTTONDOWN: {
            set_key_down(&input.any_key);
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

int main() {
    WNDCLASS window_class = {};
    window_class.style         = CS_OWNDC;
    window_class.hInstance     = GetModuleHandle(NULL);
    window_class.lpfnWndProc   = window_proc;
    window_class.lpszClassName = "WINDOW_CLASS";
    window_class.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&window_class);

    HWND window = CreateWindow(
        window_class.lpszClassName, 
        "Asteroids!", 
        WS_POPUPWINDOW, 
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (WINDOW_WIDTH  / 2), 
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (WINDOW_HEIGHT / 2), 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        NULL, 
        NULL, 
        window_class.hInstance, 
        NULL);

    ShowWindow(window, TRUE);

    init_draw(window);

    init_menu();
    init_game();

    Sprite background_sprite = load_sprite("data/background.png");

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER last_time;
    QueryPerformanceCounter(&last_time);

    while (is_running) {
        LARGE_INTEGER start_time;
        QueryPerformanceCounter(&start_time);
        
        delta_time = (float) ((double) (start_time.QuadPart - last_time.QuadPart) / (double) frequency.QuadPart);
        last_time  = start_time;

        reset_key(&input.any_key);
        reset_key(&input.left_mb);
        reset_key(&input.escape);
        reset_key(&input.space);

        MSG window_message;
        while (PeekMessage(&window_message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&window_message);
            DispatchMessage(&window_message);
        }

        draw_begin();

        float background_x = -HALF_WORLD_WIDTH;
        float background_y = -HALF_WORLD_HEIGHT;

        float background_width  = get_sprite_draw_width(&background_sprite);
        float background_height = get_sprite_draw_height(&background_sprite);

        float background_x_count = WORLD_WIDTH  / background_width;
        float background_y_count = WORLD_HEIGHT / background_height;

        for (int x = 0; x < background_x_count; x++) {
            for (int y = 0; y < background_y_count; y++) {
                Vector2 position = make_vector2(background_x, background_y);
                
                position.x += (x * background_width)  + (background_width  / 2.0f);
                position.y += (y * background_height) + (background_height / 2.0f);

                draw_sprite(&background_sprite, position, 0.0f);
            }
        }

        if (game_mode_switched) {
            switch (game_mode) {
                case GM_MENU: {
                    start_menu();
                    break;
                }
                case GM_GAME: {
                    start_game();
                    break;
                }
            }

            game_mode_switched = false;
        }

        switch (game_mode) {
            case GM_MENU: {
                update_menu();
                break;
            }
            case GM_GAME: {
                update_game();
                break;
            }
            default: {
                print("Unhandled game mode specified, going to menu...\n");
                switch_game_mode(GM_MENU);
                
                break;
            }
        }

        draw_text(format_string("%f", delta_time), make_vector2(16.0f, WINDOW_HEIGHT - 32.0f));
        draw_end();
    }

    return 0;
}