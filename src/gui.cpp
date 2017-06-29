enum struct Layout_Anchor {
    NONE,
    CENTER,
    TOP_LEFT
};

bool is_in_layout;

void begin_layout(Layout_Anchor anchor, f32 spacing = 0.0f, f32 offset_x = 0.0f, f32 offset_y = 0.0f) {
    assert(!is_in_layout);
    is_in_layout = true;
}

void end_layout() {
    assert(is_in_layout);
    is_in_layout = false;
}

void gui_text() {

}