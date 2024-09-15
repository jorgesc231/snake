#ifndef __ENGINE_UI_H__
#define __ENGINE_UI_H__

//#define IMGUI_IMPL_OPENGL_DEBUG

#include "snake.h"

void draw_gui(Renderer *renderer, Game_state *state);
void draw_score(Renderer *renderer, Game_state *state);
void draw_status(Renderer *renderer, Game_state *state);
void draw_debug_window(Renderer *renderer, Game_state *state);
void draw_debug_overlay(Renderer *renderer, Game_state *state);

void calculate_gui(Game_state *state, int32_t screen_width, int32_t screen_height);
void create_touch_controls(Game_state *state);
void draw_touch_controls(Game_state *state, Renderer *renderer);

void calculate_pad_triangles (Game_state *state, Rect rect);
void draw_pad_triangles (Game_state *state, Batch *batch, glm::vec4 color, glm::vec4 color_pressded);

void ScrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button);

inline int rect_min (int a, int b);
inline int rect_max (int a, int b);
Rect cut_left (Rect *rect, int32_t a);
Rect cut_right (Rect *rect, int32_t a);
Rect cut_top (Rect *rect, int32_t a);
Rect cut_bottom (Rect *rect, int32_t a);

Rect get_left (Rect *rect, int32_t a);
Rect get_right (Rect *rect, int32_t a);
Rect get_top (Rect *rect, int32_t a);
Rect get_bottom (Rect *rect, int32_t a);

#endif
