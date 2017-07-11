#define DRAW_GUI_BOUNDS 0

enum Gui_Entry_Type {
    GUI_ENTRY_TYPE_NONE,
    GUI_ENTRY_TYPE_PAD,
    GUI_ENTRY_TYPE_TEXT,
    GUI_ENTRY_TYPE_BUTTON,
    GUI_ENTRY_TYPE_FILL,
    GUI_ENTRY_TYPE_IMAGE,
    GUI_ENTRY_TYPE_RECTANGLE,
    GUI_ENTRY_TYPE_LAYOUT,
};

struct Gui_Entry {
    u32 id = 0;
    Gui_Entry_Type type = GUI_ENTRY_TYPE_NONE;

    f32 width  = 0.0f;
    f32 height = 0.0f;

    union {
        struct {
            f32 amount;
        }
        pad;

        struct {
            Font* font;
            utf8* value;
            f32   size;
        }
        text;

        struct {
            Font* font;
            utf8* value;
            f32   size;
        }
        button;

        struct {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        }
        fill;

        struct {
            f32 border_r;
            f32 border_g;
            f32 border_b;
            f32 border_a;

            f32 fill_r;
            f32 fill_g;
            f32 fill_b;
            f32 fill_a;

            bool fill;
        }
        rectangle;

        struct {
            Sprite* sprite;
            f32     size;
        }
        image;

        struct Gui_Layout* layout;
    };
};

enum Gui_Advance {
    GUI_ADVANCE_NONE,
    GUI_ADVANCE_VERTICAL,
    GUI_ADVANCE_HORIZONTAL
};

enum Gui_Anchor {
    GUI_ANCHOR_NONE,
    GUI_ANCHOR_CENTER,
    GUI_ANCHOR_TOP_LEFT,
    GUI_ANCHOR_TOP_RIGHT,
    GUI_ANCHOR_BOTTOM_LEFT,
    GUI_ANCHOR_BOTTOM_RIGHT,
};

struct Gui_Layout {
    Gui_Advance advance = GUI_ADVANCE_NONE;
    f32 padding = 0.0f;

    Gui_Anchor anchor = GUI_ANCHOR_NONE;
    f32 offset_x = 0.0f;
    f32 offset_y = 0.0f;

    f32 baked_width  = 0.0f;
    f32 baked_height = 0.0f;

    Gui_Layout* parent = null;
    Array<Gui_Entry> entries;
};

struct Gui_Interaction {
    u32 id = 0;
    Rectangle2 bounds;
};

struct Gui_Context {
    Matrix4 projection;
    Vector2 mouse_position;
    
    Font* default_font = null;
    
    Array<Gui_Layout*> layout_stack;
    Gui_Layout* root_layout = null;

    Gui_Interaction pending_interaction;
    Gui_Interaction hot_interaction;
    Gui_Interaction active_interaction;

    // @todo: remove this
    u32 selected_button_id = 0;
};

Gui_Context gui_context;

void gui_begin() {
    gui_context.projection = make_orthographic_matrix(0.0f, (f32) platform.window_width, (f32) platform.window_height, 0.0f);
    
    gui_context.mouse_position = unproject(
        input.mouse_x, 
        input.mouse_y, 
        platform.window_width, 
        platform.window_height, 
        gui_context.projection);

    gui_context.default_font = &font_nasalization;

    construct(&gui_context.layout_stack);
    gui_context.layout_stack.allocator = &temp_allocator;

    gui_context.root_layout = (Gui_Layout*) temp_alloc(size_of(Gui_Layout));
    construct(gui_context.root_layout);

    gui_context.root_layout->advance = GUI_ADVANCE_VERTICAL;
    gui_context.root_layout->anchor  = GUI_ANCHOR_TOP_LEFT;

    gui_context.root_layout->baked_width  = (f32) platform.window_width;
    gui_context.root_layout->baked_height = (f32) platform.window_height;

    gui_context.root_layout->entries.allocator = &temp_allocator;

    add(&gui_context.layout_stack, gui_context.root_layout);
}

void bake_layout_sizes(Gui_Layout* layout) {
    for_each (Gui_Entry* entry, &layout->entries) {
        if (entry->type == GUI_ENTRY_TYPE_LAYOUT) {
            bake_layout_sizes(entry->layout);

            entry->width  = entry->layout->baked_width;
            entry->height = entry->layout->baked_height;
        }

        if (layout != gui_context.root_layout) {
            switch (layout->advance) {
                case GUI_ADVANCE_HORIZONTAL: {
                    layout->baked_width += entry->width;
                    if (iterator.has_next) {
                        layout->baked_width += layout->padding;
                    }

                    if (entry->height > layout->baked_height) {
                        layout->baked_height = entry->height;
                    }

                    break;
                }
                case GUI_ADVANCE_VERTICAL: {
                    layout->baked_height += entry->height;
                    if (iterator.has_next) {
                        layout->baked_height += layout->padding;
                    }

                    if (entry->width > layout->baked_width) {
                        layout->baked_width = entry->width;
                    }

                    break;
                }
                invalid_default_case();
            }
        }
    }
}

void draw_layout_entries(Gui_Layout* layout, Vector2 cursor) {
    Vector2 layout_position = make_vector2(cursor.x, cursor.y - layout->baked_height);

    #if DEBUG && DRAW_GUI_BOUNDS
        set_transform(make_transform_matrix(layout_position));
        
        draw_rectangle(
            make_rectangle2(make_vector2(-5.0f, -5.0f), layout->baked_width + 10.0f, layout->baked_height + 10.0f), 
            make_color(0.0f, 0.0f, 1.0f), 
            false);
    #endif

    for_each (Gui_Entry* entry, &layout->entries) {
        if (entry->type == GUI_ENTRY_TYPE_LAYOUT && entry->layout->anchor) {
            Gui_Layout* child_layout = entry->layout;
            Vector2     child_cursor = layout_position;

            if (child_layout->anchor == GUI_ANCHOR_CENTER) {
                child_cursor.x += (layout->baked_width  / 2.0f) - (child_layout->baked_width  / 2.0f) + child_layout->offset_x;
                child_cursor.y += (layout->baked_height / 2.0f) + (child_layout->baked_height / 2.0f) + child_layout->offset_y;
            }

            if (child_layout->anchor == GUI_ANCHOR_TOP_LEFT || child_layout->anchor == GUI_ANCHOR_BOTTOM_LEFT) {
                child_cursor.x += child_layout->offset_x;
            }

            if (child_layout->anchor == GUI_ANCHOR_TOP_RIGHT || child_layout->anchor == GUI_ANCHOR_BOTTOM_RIGHT) {
                child_cursor.x += layout->baked_width - child_layout->baked_width - child_layout->offset_x;
            }

            if (child_layout->anchor == GUI_ANCHOR_TOP_LEFT || child_layout->anchor == GUI_ANCHOR_TOP_RIGHT) {
                child_cursor.y += layout->baked_height - child_layout->offset_y;
            }

            if (child_layout->anchor == GUI_ANCHOR_BOTTOM_LEFT || child_layout->anchor == GUI_ANCHOR_BOTTOM_RIGHT) {
                child_cursor.y += child_layout->baked_height + child_layout->offset_y;
            }

            draw_layout_entries(child_layout, child_cursor);
            continue;
        }

        switch (layout->advance) {
            case GUI_ADVANCE_HORIZONTAL: {
                if (layout->anchor == GUI_ANCHOR_CENTER) {
                    cursor.x = layout_position.x + (layout->baked_width / 2.0f) + layout->offset_x;
                }

                break;
            }
            case GUI_ADVANCE_VERTICAL: {
                if (layout->anchor == GUI_ANCHOR_CENTER) {
                    cursor.x = layout_position.x + (layout->baked_width / 2.0f) - (entry->width / 2.0f) + layout->offset_x;
                }

                if (layout->anchor == GUI_ANCHOR_TOP_RIGHT || layout->anchor == GUI_ANCHOR_BOTTOM_RIGHT) {
                    cursor.x = layout_position.x + layout->baked_width - entry->width;
                }

                break;
            }
        }

        cursor.y -= entry->height;

        switch (entry->type) {
            case GUI_ENTRY_TYPE_PAD: {
                break;
            }
            case GUI_ENTRY_TYPE_TEXT: {
                cursor.y += get_font_descent(entry->text.font, entry->text.size);

                set_transform(make_transform_matrix(cursor));
                draw_text(entry->text.font, entry->text.size, entry->text.value);

                cursor.y -= get_font_descent(entry->text.font, entry->text.size);

                break;
            }
            case GUI_ENTRY_TYPE_BUTTON: {
                Rectangle2 dimensions = make_rectangle2(cursor, entry->width, entry->height);
                if (contains(dimensions, gui_context.mouse_position)) {
                    gui_context.pending_interaction.id     = entry->id;
                    gui_context.pending_interaction.bounds = dimensions;
                }

                Color color = make_color(1.0f, 1.0f, 1.0f);
                if (gui_context.hot_interaction.id == entry->id) {
                    color = make_color(1.0f, 1.0f, 0.0f);
                }

                cursor.y += get_font_descent(entry->button.font, entry->button.size);

                set_transform(make_transform_matrix(cursor));
                draw_text(entry->button.font, entry->button.size, entry->button.value, color);

                cursor.y -= get_font_descent(entry->button.font, entry->button.size);

                break;
            }
            case GUI_ENTRY_TYPE_FILL: {
                set_transform(make_identity_matrix());
                
                draw_rectangle(
                    make_rectangle2(layout_position, layout->baked_width, layout->baked_height), 
                    make_color(entry->fill.r, entry->fill.g, entry->fill.b, entry->fill.a));

                break;
            }
            case GUI_ENTRY_TYPE_IMAGE: {
                set_transform(make_transform_matrix(cursor));
                draw_sprite(entry->image.sprite, entry->image.size, 1.0f, false);

                break;
            }
            case GUI_ENTRY_TYPE_RECTANGLE: {
                set_transform(make_transform_matrix(cursor));

                Rectangle2 rectangle = make_rectangle2(make_vector2(0.0f, 0.0f), entry->width, entry->height);

                if (entry->rectangle.fill) {
                    draw_rectangle(
                        rectangle, 
                        make_color(entry->rectangle.fill_r, entry->rectangle.fill_g, entry->rectangle.fill_b, entry->rectangle.fill_a), 
                        true);
                }

                draw_rectangle(
                    rectangle, 
                    make_color(entry->rectangle.border_r, entry->rectangle.border_g, entry->rectangle.border_b, entry->rectangle.border_a), 
                    false);

                break;
            }
            case GUI_ENTRY_TYPE_LAYOUT: {
                Gui_Layout* child_layout = entry->layout;
                Vector2     child_cursor = cursor;

                child_cursor.x += child_layout->offset_x;
                child_cursor.y += entry->height;

                draw_layout_entries(child_layout, child_cursor);
                break;
            }
        }

        #if DEBUG && DRAW_GUI_BOUNDS
            if (entry->type != GUI_ENTRY_TYPE_LAYOUT) {
                set_transform(make_identity_matrix());
                draw_rectangle(make_rectangle2(cursor, entry->width, entry->height), make_color(0.0f, 1.0f, 0.0f), false);
            }
        #endif

        cursor.y += entry->height;

        switch (layout->advance) {
            case GUI_ADVANCE_HORIZONTAL: {
                cursor.x += entry->width;
                cursor.x += layout->padding;

                break;
            }
            case GUI_ADVANCE_VERTICAL: {
                cursor.y -= entry->height;
                cursor.y -= layout->padding;

                break;
            }
            invalid_default_case();
        }
    }
}

void gui_end() {
    bake_layout_sizes(gui_context.root_layout);

    set_projection(gui_context.projection);
    draw_layout_entries(gui_context.root_layout, make_vector2(0.0f, gui_context.root_layout->baked_height));

    gui_context.selected_button_id = 0;

    if (gui_context.active_interaction.id) {
        if (input.mouse_left.up) {
            if (contains(gui_context.active_interaction.bounds, gui_context.mouse_position)) {
                gui_context.selected_button_id = gui_context.active_interaction.id;
            }
            
            gui_context.active_interaction.id = 0;
        }
    }
    else {
        gui_context.hot_interaction = gui_context.pending_interaction;
        
        if (input.mouse_left.down) {
            if (gui_context.hot_interaction.id) {
                gui_context.active_interaction = gui_context.hot_interaction;
            }
        }
    }

    gui_context.pending_interaction.id = 0;
}

Gui_Layout* get_current_layout() {
    return gui_context.layout_stack[gui_context.layout_stack.count - 1];
}

void begin_layout(Gui_Advance advance, f32 padding, Gui_Anchor anchor = GUI_ANCHOR_NONE, f32 offset_x = 0.0f, f32 offset_y = 0.0f) {
    Gui_Layout* layout = (Gui_Layout*) temp_alloc(size_of(Gui_Layout));
    construct(layout);

    layout->advance  = advance;
    layout->padding  = padding;
    layout->anchor   = anchor;
    layout->offset_x = offset_x;
    layout->offset_y = offset_y;

    layout->parent = get_current_layout();
    layout->entries.allocator = &temp_allocator;

    Gui_Entry entry;
    
    entry.type   = GUI_ENTRY_TYPE_LAYOUT;
    entry.layout = layout;

    add(&layout->parent->entries, entry);
    add(&gui_context.layout_stack, layout);
}

void begin_layout(Gui_Advance advance = GUI_ADVANCE_VERTICAL, Gui_Anchor anchor = GUI_ANCHOR_NONE, f32 offset_x = 0.0f, f32 offset_y = 0.0f) {
    begin_layout(advance, 0.0f, anchor, offset_x, offset_y);
}

void end_layout() {
    remove(&gui_context.layout_stack, gui_context.layout_stack.count - 1);
}

void gui_pad(f32 padding) {
    Gui_Entry entry;

    entry.type       = GUI_ENTRY_TYPE_PAD;
    entry.pad.amount = padding;

    switch (get_current_layout()->advance) {
        case GUI_ADVANCE_HORIZONTAL: {
            entry.width += entry.pad.amount;
            break;
        }
        case GUI_ADVANCE_VERTICAL: {
            entry.height += entry.pad.amount;
            break;
        }
        invalid_default_case();
    }

    add(&get_current_layout()->entries, entry);
}

void gui_text(Font* font, utf8* text, f32 size) {
    Gui_Entry entry;
    entry.type = GUI_ENTRY_TYPE_TEXT;
    
    entry.width  = get_text_width(font, size, text);
    entry.height = get_font_ascent(font, size) + get_font_descent(font, size);

    entry.text.font  = font;
    entry.text.value = text;
    entry.text.size  = size;

    add(&get_current_layout()->entries, entry);
}

void gui_text(utf8* text, f32 size) {
    gui_text(gui_context.default_font, text, size);
}

bool gui_button(u32 id, Font* font, utf8* text, f32 size) {
    Gui_Entry entry;
    
    entry.id   = id + hash(text);
    entry.type = GUI_ENTRY_TYPE_BUTTON;
    
    entry.width  = get_text_width(font, size, text);
    entry.height = get_font_ascent(font, size) + get_font_descent(font, size);

    entry.button.font  = font;
    entry.button.value = text;
    entry.button.size  = size;

    add(&get_current_layout()->entries, entry);

    return entry.id == gui_context.selected_button_id;
}

bool gui_button(u32 id, utf8* text, f32 size) {
    return gui_button(id, gui_context.default_font, text, size);
}

bool gui_button(utf8* text, f32 size) {
    return gui_button(0, text, size);
}

void gui_fill(Color color) {
    Gui_Entry entry;
    entry.type = GUI_ENTRY_TYPE_FILL;

    entry.fill.r = color.r;
    entry.fill.g = color.g;
    entry.fill.b = color.b;
    entry.fill.a = color.a;

    add(&get_current_layout()->entries, entry);
}

void gui_image(Sprite* sprite, f32 size) {
    Gui_Entry entry;
    entry.type = GUI_ENTRY_TYPE_IMAGE;

    entry.width  = get_sprite_width(sprite, size);
    entry.height = size;

    entry.image.sprite = sprite;
    entry.image.size   = size;

    add(&get_current_layout()->entries, entry);
}

void gui_rectangle(f32 width, f32 height, Color border_color, Color fill_color, bool fill = true) {
    Gui_Entry entry;
    entry.type = GUI_ENTRY_TYPE_RECTANGLE;

    entry.width  = width;
    entry.height = height;

    entry.rectangle.border_r    = border_color.r;
    entry.rectangle.border_g    = border_color.g;
    entry.rectangle.border_b    = border_color.b;
    entry.rectangle.border_a    = border_color.a;

    entry.rectangle.fill_r    = fill_color.r;
    entry.rectangle.fill_g    = fill_color.g;
    entry.rectangle.fill_b    = fill_color.b;
    entry.rectangle.fill_a    = fill_color.a;

    entry.rectangle.fill = fill;

    add(&get_current_layout()->entries, entry);
}

void gui_rectangle(f32 width, f32 height, Color border_color) {
    gui_rectangle(width, height, border_color, make_color(0.0f, 0.0f, 0.0f), false);
}