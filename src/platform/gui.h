#ifndef GUI_H
#define GUI_H

#include <string>
#include <stdint.h>

class GuiRenderer {
public:
    GuiRenderer();
    ~GuiRenderer();

    void init();
    void render(int mouse_x, int mouse_y, bool mouse_click);
    bool handle_click(int mouse_x, int mouse_y);
    void draw_string(const std::string& str, float x, float y, float scale, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

private:
    void draw_rect(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void draw_border(float x, float y, float w, float h, float thickness, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void draw_line(float x1, float y1, float x2, float y2, float thickness, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void draw_char(char c, float x, float y, float scale, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    bool show_about_modal = false;
    bool is_music_muted = false;
};

#endif
