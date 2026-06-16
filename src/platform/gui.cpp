#include "platform/gui.h"
#include <GL/gl.h>
#include <iostream>
#include <cstring>

// Standard 8x8 font bitmap mapping ASCII 32 (' ') to 127
static const uint8_t font_8x8[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00}, // !
    {0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x36, 0x36, 0x7f, 0x36, 0x7f, 0x36, 0x36, 0x00}, // #
    {0x0c, 0x3f, 0x0c, 0x0e, 0x3c, 0x0c, 0x3e, 0x0c}, // $
    {0x00, 0x66, 0x66, 0x30, 0x18, 0x0c, 0x66, 0x66}, // %
    {0x3c, 0x66, 0x3c, 0x38, 0x67, 0x66, 0x3f, 0x00}, // &
    {0x06, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // '
    {0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c}, // (
    {0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30}, // )
    {0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00}, // *
    {0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30}, // ,
    {0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // .
    {0x00, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00}, // /
    {0x3e, 0x63, 0x67, 0x6f, 0x7b, 0x63, 0x3e, 0x00}, // 0
    {0x0c, 0x1c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3e, 0x00}, // 1
    {0x3e, 0x63, 0x06, 0x1c, 0x30, 0x60, 0x7f, 0x00}, // 2
    {0x7f, 0x06, 0x0c, 0x1c, 0x06, 0x63, 0x3e, 0x00}, // 3
    {0x06, 0x0f, 0x1b, 0x33, 0x7f, 0x03, 0x03, 0x00}, // 4
    {0x7f, 0x60, 0x7e, 0x03, 0x03, 0x63, 0x3e, 0x00}, // 5
    {0x1c, 0x30, 0x60, 0x7e, 0x63, 0x63, 0x3e, 0x00}, // 6
    {0x7f, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x30, 0x00}, // 7
    {0x3e, 0x63, 0x63, 0x3e, 0x63, 0x63, 0x3e, 0x00}, // 8
    {0x3e, 0x63, 0x63, 0x7f, 0x03, 0x06, 0x3c, 0x00}, // 9
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // :
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00}, // ;
    {0x06, 0x0c, 0x18, 0x30, 0x18, 0x0c, 0x06, 0x00}, // <
    {0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00}, // =
    {0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00}, // >
    {0x3e, 0x63, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00}, // ?
    {0x3e, 0x63, 0x6f, 0x6b, 0x6b, 0x60, 0x3e, 0x00}, // @
    {0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00}, // A
    {0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00}, // B
    {0x3e, 0x63, 0x60, 0x60, 0x60, 0x63, 0x3e, 0x00}, // C
    {0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00}, // D
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x7e, 0x00}, // E
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00}, // F
    {0x3e, 0x63, 0x60, 0x6e, 0x63, 0x63, 0x3e, 0x00}, // G
    {0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00}, // H
    {0x3e, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3e, 0x00}, // I
    {0x1f, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3c, 0x00}, // J
    {0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00}, // K
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7f, 0x00}, // L
    {0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00}, // M
    {0x63, 0x63, 0x67, 0x6f, 0x7b, 0x73, 0x63, 0x00}, // N
    {0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00}, // O
    {0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00}, // P
    {0x3e, 0x63, 0x63, 0x63, 0x6b, 0x66, 0x3d, 0x00}, // Q
    {0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00}, // R
    {0x3e, 0x63, 0x38, 0x0e, 0x07, 0x63, 0x3e, 0x00}, // S
    {0x7f, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x00}, // T
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, // U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // V
    {0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00}, // W
    {0x63, 0x63, 0x36, 0x1c, 0x36, 0x63, 0x63, 0x00}, // X
    {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00}, // Y
    {0x7f, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7f, 0x00}, // Z
    {0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00}, // [
    {0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x00}, // backslash
    {0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00}, // ]
    {0x08, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // ^
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff}, // _
    {0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00}, // `
    {0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3b, 0x00}, // a
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00}, // b
    {0x00, 0x00, 0x3c, 0x60, 0x60, 0x62, 0x3c, 0x00}, // c
    {0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x00}, // d
    {0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00}, // e
    {0x0e, 0x18, 0x3e, 0x18, 0x18, 0x18, 0x18, 0x00}, // f
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c}, // g
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // h
    {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00}, // i
    {0x06, 0x00, 0x0e, 0x06, 0x06, 0x06, 0x06, 0x3c}, // j
    {0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00}, // k
    {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00}, // l
    {0x00, 0x00, 0x66, 0x7f, 0x6b, 0x6b, 0x63, 0x00}, // m
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // n
    {0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00}, // o
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60}, // p
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06}, // q
    {0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00}, // r
    {0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00}, // s
    {0x18, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x0d, 0x06}, // t
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3b, 0x00}, // u
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // v
    {0x00, 0x00, 0x63, 0x6b, 0x6b, 0x7f, 0x36, 0x00}, // w
    {0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00}, // x
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c}, // y
    {0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00}, // z
    {0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00}, // {
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // |
    {0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00}, // }
    {0x3b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ~
    {0x1c, 0x36, 0x36, 0x1c, 0x00, 0x00, 0x00, 0x00}  // copyright/bullet
};

// Window constants
static const int WIN_W = 1920;
static const int WIN_H = 1080;
static const int SIDEBAR_W = 480;  // sidebar width at HD scale
static const int GAME_AREA_W = WIN_W; // game fills full width when GUI hidden

GuiRenderer::GuiRenderer() {}
GuiRenderer::~GuiRenderer() {}

void GuiRenderer::init() {
    show_about_modal = false;
    is_music_muted = false;
}

void GuiRenderer::draw_rect(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void GuiRenderer::draw_border(float x, float y, float w, float h, float thickness, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glColor4ub(r, g, b, a);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void GuiRenderer::draw_line(float x1, float y1, float x2, float y2, float thickness, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glColor4ub(r, g, b, a);
    glLineWidth(thickness);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void GuiRenderer::draw_char(char c, float x, float y, float scale, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int idx = (uint8_t)c - 32;
    if (idx < 0 || idx >= 96) return;
    const uint8_t* glyph = font_8x8[idx];
    
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    for (int row = 0; row < 8; ++row) {
        uint8_t row_data = glyph[row];
        for (int col = 0; col < 8; ++col) {
            if (row_data & (1 << (7 - col))) {
                float px = x + col * scale;
                float py = y + (7 - row) * scale;
                glVertex2f(px, py);
                glVertex2f(px + scale, py);
                glVertex2f(px + scale, py + scale);
                glVertex2f(px, py + scale);
            }
        }
    }
    glEnd();
}

void GuiRenderer::draw_string(const std::string& str, float x, float y, float scale, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    float cur_x = x;
    for (char c : str) {
        if (c == '\n') {
            y -= 12.0f * scale;
            cur_x = x;
            continue;
        }
        draw_char(c, cur_x, y, scale, r, g, b, a);
        cur_x += 8.0f * scale;
    }
}

void GuiRenderer::render(int mouse_x, int mouse_y, bool mouse_click) {
    // Save current OpenGL states
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Set viewport to full window
    glViewport(0, 0, WIN_W, WIN_H);
    
    // Set 2D orthographic projection over full window
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIN_W, 0, WIN_H, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable elements that conflict with GUI rendering
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // --- Top Menu Bar (like yuzu/Citra) ---
    float bar_h = 36;
    float bar_y = WIN_H - bar_h;
    
    // Dark translucent menu bar
    draw_rect(0, bar_y, WIN_W, bar_h, 18, 18, 24, 240);
    draw_line(0, bar_y, WIN_W, bar_y, 1.0f, 0, 168, 204, 120);
    
    // Menu headings
    float menu_x = 16;
    float text_y = bar_y + 10;
    float heading_scale = 1.8f;
    
    draw_string("File", menu_x, text_y, heading_scale, 220, 225, 235, 255);
    menu_x += 80;
    draw_string("Emulation", menu_x, text_y, heading_scale, 220, 225, 235, 255);
    menu_x += 160;
    draw_string("Help", menu_x, text_y, heading_scale, 220, 225, 235, 255);
    menu_x += 80;
    draw_string("About", menu_x, text_y, heading_scale, 220, 225, 235, 255);
    
    // Right-aligned status
    draw_string("F1: Toggle GUI", WIN_W - 240, text_y, 1.3f, 100, 110, 130, 200);
    
    // --- Right Sidebar Panel ---
    float sb_x = WIN_W - SIDEBAR_W;
    draw_rect(sb_x, 0, SIDEBAR_W, WIN_H - bar_h, 20, 24, 33, 230);
    draw_line(sb_x, 0, sb_x, WIN_H - bar_h, 2.0f, 0, 168, 204, 200);
    
    // Title header in sidebar
    float sy = WIN_H - bar_h - 50;
    draw_string("SWORDIGO", sb_x + 30, sy, 3.5f, 255, 255, 255, 255);
    sy -= 35;
    draw_string("DESKTOP", sb_x + 30, sy, 2.5f, 0, 168, 204, 255);
    sy -= 22;
    draw_string("Remastered Edition", sb_x + 30, sy, 1.5f, 120, 120, 140, 255);
    
    // Separator
    sy -= 15;
    draw_line(sb_x + 20, sy, WIN_W - 20, sy, 1.0f, 40, 50, 70, 255);
    
    // Controls Section
    sy -= 25;
    draw_string("CONTROLS", sb_x + 30, sy, 1.8f, 0, 168, 204, 255);
    sy -= 25;
    
    auto draw_row = [&](const std::string& key, const std::string& desc) {
        draw_string(key, sb_x + 30, sy, 1.3f, 255, 255, 255, 200);
        draw_string(desc, sb_x + 200, sy, 1.3f, 180, 190, 210, 255);
        sy -= 22;
    };
    
    draw_row("Arrow Keys", "Move");
    draw_row("Space / W", "Jump");
    draw_row("J / Z", "Attack");
    draw_row("K / X", "Magic");
    draw_row("I", "Use Item");
    draw_row("Escape", "Game Menu");
    draw_row("F1", "Toggle GUI");
    
    // Separator
    sy -= 10;
    draw_line(sb_x + 20, sy, WIN_W - 20, sy, 1.0f, 40, 50, 70, 255);
    
    // Buttons Section
    sy -= 30;
    float btn_w = SIDEBAR_W - 60;
    float btn_h = 40;
    
    auto render_button = [&](float bx, float by, float bw, float bh, const std::string& label, bool active = false) {
        bool hover = (mouse_x >= bx && mouse_x < bx + bw && mouse_y >= by && mouse_y < by + bh);
        uint8_t br = 30, bg = 41, bb = 59;
        if (hover) { br = 51; bg = 65; bb = 85; }
        if (active) { br = 0; bg = 120; bb = 160; }
        draw_rect(bx, by, bw, bh, br, bg, bb, 255);
        draw_border(bx, by, bw, bh, 1.0f, 0, 168, 204, hover ? 255 : 120);
        
        float text_w = label.length() * 8.0f * 1.3f;
        float text_x = bx + (bw - text_w) / 2.0f;
        float text_y = by + (bh - 8.0f * 1.3f) / 2.0f;
        draw_string(label, text_x, text_y, 1.3f, 255, 255, 255, 255);
    };
    
    render_button(sb_x + 30, sy, btn_w, btn_h, "Pause Emulation");
    sy -= 50;
    render_button(sb_x + 30, sy, btn_w, btn_h, "Restart Engine");
    sy -= 50;
    render_button(sb_x + 30, sy, btn_w, btn_h, is_music_muted ? "Unmute Music" : "Mute Music");
    sy -= 50;
    render_button(sb_x + 30, sy, btn_w, btn_h, "About", show_about_modal);
    
    // Footer
    draw_string("QuantumCreeper Labs", sb_x + 100, 25, 1.3f, 90, 100, 120, 255);
    draw_string("v0.2.0-alpha", sb_x + 140, 8, 1.0f, 60, 70, 90, 200);
    
    // --- Bottom Status Bar ---
    float status_h = 28;
    draw_rect(0, 0, WIN_W - SIDEBAR_W, status_h, 18, 18, 24, 220);
    draw_line(0, status_h, WIN_W - SIDEBAR_W, status_h, 1.0f, 0, 168, 204, 80);
    draw_string("Swordigo Desktop | ARM Emulation | 960x544 -> 1920x1080", 12, 8, 1.2f, 140, 150, 170, 200);
    
    // --- About Modal ---
    if (show_about_modal) {
        // Darken game area
        draw_rect(0, 0, WIN_W - SIDEBAR_W, WIN_H - bar_h, 0, 0, 0, 180);
        
        float mx = 300, my = 250, mw = 800, mh = 450;
        draw_rect(mx, my, mw, mh, 15, 23, 42, 248);
        draw_border(mx, my, mw, mh, 2.0f, 0, 168, 204, 255);
        
        draw_string("ABOUT SWORDIGO DESKTOP", mx + 40, my + mh - 50, 2.2f, 0, 168, 204, 255);
        draw_line(mx + 40, my + mh - 65, mx + mw - 40, my + mh - 65, 1.0f, 50, 70, 100, 255);
        
        draw_string("(C) 2026 QuantumCreeper Labs\n\n"
                    "This project brings Touchfoo's masterpiece \"Swordigo\"\n"
                    "to Linux desktop using dynamic ARM emulation.\n\n"
                    "Acknowledgements:\n"
                    "  - Rinnegatamante for the Vita port reference\n"
                    "  - Touchfoo for creating Swordigo\n\n"
                    "License: MIT", 
                    mx + 40, my + mh - 95, 1.4f, 220, 230, 245, 255);
        
        render_button(mx + mw - 200, my + 30, 160, 40, "Close");
    }
    
    // Restore OpenGL state
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

bool GuiRenderer::handle_click(int mouse_x, int mouse_y) {
    float bar_h = 36;
    float bar_y = WIN_H - bar_h;
    float sb_x = WIN_W - SIDEBAR_W;
    
    // About modal close button
    if (show_about_modal) {
        float mx = 300, my = 250, mw = 800, mh = 450;
        float bx = mx + mw - 200, by = my + 30;
        if (mouse_x >= bx && mouse_x < bx + 160 && mouse_y >= by && mouse_y < by + 40) {
            show_about_modal = false;
            return true;
        }
        if (mouse_x >= mx && mouse_x < mx + mw && mouse_y >= my && mouse_y < my + mh)
            return true;
    }
    
    // Top menu bar clicks
    if (mouse_y >= bar_y && mouse_y < WIN_H) {
        return true; // swallow menu bar clicks
    }
    
    // Sidebar clicks
    if (mouse_x >= sb_x) {
        // Calculate button positions matching render()
        float sy = WIN_H - bar_h - 50 - 35 - 22 - 15 - 25 - 25 - (22 * 7) - 10 - 30;
        float btn_w = SIDEBAR_W - 60;
        float btn_h = 40;
        
        // Pause Emulation
        if (mouse_x >= sb_x + 30 && mouse_x < sb_x + 30 + btn_w && mouse_y >= sy && mouse_y < sy + btn_h) {
            std::cout << "[GUI] Pause Emulation clicked" << std::endl;
            return true;
        }
        sy -= 50;
        
        // Restart Engine
        if (mouse_x >= sb_x + 30 && mouse_x < sb_x + 30 + btn_w && mouse_y >= sy && mouse_y < sy + btn_h) {
            std::cout << "[GUI] Restart Engine clicked" << std::endl;
            return true;
        }
        sy -= 50;
        
        // Mute Music
        if (mouse_x >= sb_x + 30 && mouse_x < sb_x + 30 + btn_w && mouse_y >= sy && mouse_y < sy + btn_h) {
            is_music_muted = !is_music_muted;
            std::cout << "[GUI] Mute Music -> " << is_music_muted << std::endl;
            return true;
        }
        sy -= 50;
        
        // About
        if (mouse_x >= sb_x + 30 && mouse_x < sb_x + 30 + btn_w && mouse_y >= sy && mouse_y < sy + btn_h) {
            show_about_modal = !show_about_modal;
            std::cout << "[GUI] About -> " << show_about_modal << std::endl;
            return true;
        }
        
        return true; // swallow all sidebar clicks
    }
    
    // Bottom status bar
    if (mouse_y < 28 && mouse_x < sb_x) {
        return true;
    }
    
    return false;
}
