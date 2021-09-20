#pragma once
#include <kinc/global.h>
#include <kinc/math/vector.h>
#include <kinc/math/matrix.h>
#include <kinc/graphics4/texture.h>

#ifdef __cplusplus
extern "C" {
#endif
void kinc_g2_init(void);
void kinc_g2_destroy(void);
void kinc_g2_begin(void);
void kinc_g2_end(void);
void kinc_g2_clear(uint32_t color);
void kinc_g2_set_color(uint32_t color);
void kinc_g2_draw_rect(float x, float y, float width, float height, float strength);
void kinc_g2_fill_rect(float x, float y, float width, float height);
void kinc_g2_fill_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
void kinc_g2_draw_string(const char* text, float x, float y);
void kinc_g2_draw_line(float x1, float y1, float x2, float y2, float strength);
void kinc_g2_draw_image(kinc_g4_texture_t* img, float x, float y);
void kinc_g2_draw_scaled_subImage(kinc_g4_texture_t* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_reset_transform();
void kinc_g2_pushTransformation(kinc_matrix3x3_t* trans);
kinc_matrix3x3_t* kinc_g2_popTransformation();
void kinc_g2_translate(kinc_vector2_t pos);
void kinc_g2_set_rotation(float angle, float centerx, float centery);
const char* kinc_g2_get_font();
int kinc_g2_get_font_size();
void kinc_g2_set_font(const char* font,size_t size);
void kinc_g2_scissor(int x, int y, int w, int h);

void kinc_g2_ext_draw_arc(float cx, float cy, float radius, float sAngle, float eAngle, float strength, bool ccw, int segments);
void kinc_g2_ext_fill_arc(float cx, float cy, float radius, float sAngle, float eAngle, float strength, bool ccw, int segments);
void kinc_g2_ext_draw_circle(float cx, float cy, float radius, float strength, int segments);
void kinc_g2_ext_fill_circle(float cx, float cy, float radius, float strength, int segments);
void kinc_g2_ext_drawInnerLine(float x1, float y1, float  x2, float y2, float strength);
void kinc_g2_ext_draw_polygon(float x, float y,kinc_vector2_t* vertices, size_t v_len, float strength);
void kinc_g2_ext_fill_polygon(float x, float y,kinc_vector2_t* vertices, size_t v_len);
void kinc_g2_ext_draw_cubicBezier(float* x, float* y, int segments, float strength);
void kinc_g2_ext_draw_cubicBezierPath(float* x, float* y, int point_count, int segments, float strength);
#ifdef __cplusplus
}
#endif
