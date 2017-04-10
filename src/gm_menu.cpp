struct {
    char*   message_text;
    Vector2 message_position;
}
menu;

void init_menu() {
    menu.message_text = "Press any key to play";

    float half_draw_width = get_text_draw_width(menu.message_text) / 2.0f;
    menu.message_position = make_vector2(HALF_WINDOW_WIDTH - half_draw_width, HALF_WINDOW_HEIGHT);
}

void update_menu() {
    if (input.escape.down) {
        is_running = false;
    }
    
    if (input.any_key.down) {
        switch_game_mode(GM_GAME);
    }

    draw_text(menu.message_text, menu.message_position);
}