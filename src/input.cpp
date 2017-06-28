#include "xinput.h"
#pragma comment(lib, "xinput.lib")

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

    u32 mouse_x = 0;
    u32 mouse_y = 0;

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

void update_input() {
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
    ScreenToClient(window, &cursor_position);

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