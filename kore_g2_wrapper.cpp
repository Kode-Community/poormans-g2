
#include <string.h>
#if KORE
#include "assert.h"
#else
#define assert(value)
#endif
#include <kinc/window.h>
#include <kinc/color.h>
#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>
#include <kinc/log.h>

#include <Kore/Graphics2/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics2/Kravur.h>
#include <Kore/IO/FileReader.h>

#include "kore_g2_wrapper.h"

#define FONT_MAX 512
using namespace Kore::Graphics2;

static Graphics2* g2 = NULL;
static Kore::Graphics4::Texture** tex;
static uint32_t clearColor = KINC_COLOR_BLACK;

static kinc_matrix3x3_t transformation;
static kinc_matrix3x3_t* transformations;
static int transformationIndex = 0;

static int image_count = 0;
static int max_images = 2048;
static char fontName[256] = {0};
static kinc_matrix3x3_t ident = { 0 };
static void kinc_matrix3x3_set_from(kinc_matrix3x3_t* matrix, kinc_matrix3x3_t* from) {
    memcpy(matrix, from, sizeof(kinc_matrix3x3_t));
}
void kinc_g2_init(){
    tex = (Kore::Graphics4::Texture**)malloc(sizeof(Kore::Graphics4::Texture*) * max_images);
    assert(tex != NULL);
    for (int i = 0; i < max_images; ++i) {
        tex[i] = new Kore::Graphics4::Texture(32, 32, Kore::Graphics1::Image::RGBA32);
    }
    g2 = new Graphics2(kinc_window_width(0),kinc_window_height(0));
    kinc_g2_set_font("font_default",12);

    transformations = (kinc_matrix3x3_t*)malloc(sizeof(kinc_matrix3x3_t) * 1024);
    assert(transformations != NULL);
    ident = kinc_matrix3x3_identity();
    transformations[0] = ident;
}

void kinc_g2_destroy(void) {
    assert(g2 != NULL);
    delete g2;
    if (tex != NULL) {
        for (int i = 0; i < max_images; ++i) {
            if (tex[i] != NULL)
                free(tex[i]);
        }
    }
    free(transformations);
}

void kinc_g2_begin() {
    Kore::Graphics4::begin();
    assert(g2 != NULL);
    g2->begin(false,-1,-1,true, clearColor);
    image_count = 0;
}

void kinc_g2_end(void) {
    g2->end();
    Kore::Graphics4::end();
    Kore::Graphics4::swapBuffers();
    image_count = 0;
}

void kinc_g2_clear(uint32_t color) {
    clearColor = color;
    g2->clear(color);
}

void kinc_g2_set_color(uint32_t color) {
    g2->setFontColor(color);
    g2->setColor(color);
}

void kinc_g2_draw_rect(float x, float y, float width, float height, float strength) {
    g2->drawRect(x, y, width, height, strength);
}

void kinc_g2_fill_rect(float x, float y, float width, float height)
{
    g2->fillRect(x, y, width, height);
}

void kinc_g2_fill_triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
    g2->fillTriangle(x1, y1, x2, y2, x3, y3);
}

void kinc_g2_draw_string(const char* text, float x, float y) {
    g2->drawString(text, x, y);
}

void kinc_g2_draw_line(float x1, float y1, float x2, float y2, float strength)
{
    g2->drawLine(x1, y1, x2, y2, strength);
}

void kinc_g2_draw_image(kinc_g4_texture_t* img, float x, float y)
{
    kinc_g2_draw_scaled_subImage(img, 0, 0, img->tex_width, img->tex_height, x, y, img->tex_width, img->tex_height);
}

void kinc_g2_draw_scaled_subImage(kinc_g4_texture_t* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh)
{
    kinc_g2_set_color(KINC_COLOR_WHITE);
    tex[image_count]->kincTexture = *img;
    tex[image_count]->width = img->tex_width;
    tex[image_count]->height = img->tex_height;
    tex[image_count]->texWidth = img->tex_width;
    tex[image_count]->texHeight = img->tex_height;
    g2->drawScaledSubImage(tex[image_count], sx, sy, sw, sh, dx, dy, dw, dh);
    image_count++;
    if (image_count > max_images) {
        kinc_log(KINC_LOG_LEVEL_ERROR, "Wow somebody likes drawing images...");
        exit(1);
    }
}

void kinc_g2_reset_transform() {
    g2->transformation = Kore::mat3::Identity();
}


static void setMat3(Kore::mat3* mat, kinc_matrix3x3_t* mat2) {
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            mat->matrix[x][y] = kinc_matrix3x3_get(mat2,x,y);
        }
    }
}
void kinc_g2_pushTransformation(kinc_matrix3x3_t* trans) {
    transformationIndex++;
    kinc_matrix3x3_set_from(&transformations[transformationIndex],trans);
    Kore::mat3 last = Kore::mat3::Identity();
    setMat3(&(last), &transformations[transformationIndex]);
    g2->transformation = last * g2->transformation;

}

kinc_matrix3x3_t* kinc_g2_popTransformation()
{
    transformationIndex--;
    setMat3(&(g2->transformation), &transformations[transformationIndex]);
    return &transformations[transformationIndex+1];
}

void kinc_g2_translate(kinc_vector2_t pos) {
    kinc_matrix3x3_t t = kinc_matrix3x3_translation(pos.x, pos.y);
    kinc_g2_pushTransformation(&t);
}
void kinc_g2_set_rotation(float angle, float centerx, float centery)
{
    kinc_matrix3x3_t rot = kinc_matrix3x3_rotation_z(angle);
    kinc_matrix3x3_t trans = kinc_matrix3x3_translation(centerx,centery);
    kinc_matrix3x3_t neg_trans = kinc_matrix3x3_translation(-centerx, -centery);
    kinc_matrix3x3_t res = kinc_matrix3x3_multiply(&trans, &rot);
    kinc_matrix3x3_t t = kinc_matrix3x3_multiply(&res, &neg_trans);
    kinc_g2_pushTransformation(&t);
}

const char* kinc_g2_get_font()
{
    return fontName;
}

int kinc_g2_get_font_size() {
    return g2->getFontSize();
}

void kinc_g2_set_font(const char* font,size_t size)
{
    Kore::Kravur* f = Kore::Kravur::load(font, FontStyle(), size);
    if (f != NULL) {
        strcpy(fontName, font);
        g2->setFont(f);
    }
}

void kinc_g2_scissor(int x, int y, int w, int h) {
    g2->scissor(x,y,w,h);
}

static void vec2_sub(kinc_vector2_t* v1, kinc_vector2_t* v2, kinc_vector2_t* res) {
    res->x = v1->x - v2->x;
    res->y = v1->y - v2->y;
}
static float vec2_get_length(kinc_vector2_t* v) {
    return kinc_sqrt(v->x * v->x + v->y * v->y);
}
static void vec2_set_length(kinc_vector2_t* v,float length){
        float currentLength = vec2_get_length(v);
        if (currentLength == 0)
            return;
        float mul = length / currentLength;
        v->x *= mul;
        v->y *= mul;
}

/**
     * Draws a arc.
     * @param	ccw (optional) Specifies whether the drawing should be counterclockwise.
     * @param	segments (optional) The amount of lines that should be used to draw the arc.
     */
void kinc_g2_ext_draw_arc(float cx,float cy,float radius,float sAngle,float eAngle,float strength,bool ccw,int segments){

    sAngle = kinc_mod(sAngle, (KINC_PI * 2));
    eAngle = kinc_mod(eAngle, (KINC_PI * 2));

    if (ccw) {
        if (eAngle > sAngle)
            eAngle -= KINC_PI * 2;
    }
    else if (eAngle < sAngle)
        eAngle += KINC_PI * 2;

    radius += strength / 2;
    if (segments <= 0)
        segments = kinc_floor(10 * kinc_sqrt(radius));

    float theta = (eAngle - sAngle) / segments;
    float c = kinc_cos(theta);
    float s = kinc_sin(theta);

    float x = kinc_cos(sAngle) * radius;
    float y = kinc_sin(sAngle) * radius;

    for (int n = 0; n < segments;++n) {
        float px = x + cx;
        float py = y + cy;

        float t = x;
        x = c * x - s * y;
        y = c * y + s * t;

        kinc_g2_ext_drawInnerLine(x + cx, y + cy, px, py, strength);
    }
}

/**
 * Draws a filled arc.
 * @param	ccw (optional) Specifies whether the drawing should be counterclockwise.
 * @param	segments (optional) The amount of lines that should be used to draw the arc.
 */
void kinc_g2_ext_fill_arc(float cx, float cy, float radius, float sAngle, float eAngle, float strength, bool ccw, int segments){


    sAngle = kinc_mod(sAngle,(KINC_PI * 2));
    eAngle = kinc_mod(eAngle,(KINC_PI * 2));

    if (ccw) {
        if (eAngle > sAngle)
            eAngle -= KINC_PI * 2;
    }
    else if (eAngle < sAngle)
        eAngle += KINC_PI * 2;

    if (segments <= 0)
        segments = kinc_floor(10 * kinc_sqrt(radius));

    float theta = (eAngle - sAngle) / segments;
    float c = kinc_cos(theta);
    float s = kinc_sin(theta);

    float x = kinc_cos(sAngle) * radius;
    float y = kinc_sin(sAngle) * radius;
    float sx = x + cx;
    float sy = y + cy;

    for (int n = 0; n < segments; ++n){
        float px = x + cx;
        float py = y + cy;

        float t = x;
        x = c * x - s * y;
        y = c * y + s * t;

        kinc_g2_fill_triangle(px, py, x + cx, y + cy, sx, sy);
    }
}

void kinc_g2_ext_draw_circle(float cx, float cy, float radius, float strength, int segments) {
    if (strength <= 0)
        strength = 1;
    radius += strength / 2;

    if (segments <= 0)
        segments = kinc_floor(10 * kinc_sqrt(radius));

    float theta = 2 * KINC_PI / segments;
    float c = kinc_cos(theta);
    float s = kinc_sin(theta);

    float x = radius;
    float y = 0.0;

    for (int n = 0; n < segments; ++n) {
        float px = x + cx;
        float py = y + cy;

        float t = x;
        x = c * x - s * y;
        y = c * y + s * t;
        kinc_g2_ext_drawInnerLine(x + cx, y + cy, px, py, strength);
    }
}

void kinc_g2_ext_fill_circle(float cx, float cy, float radius, float strength, int segments) {
        if (strength <= 0)
            strength = 1;
        if (segments <= 0) {
            segments = kinc_floor(10 * kinc_sqrt(radius));
        }

        float theta = 2 * KINC_PI / segments;
        float c = kinc_cos(theta);
        float s = kinc_sin(theta);

        float x = radius;
        float y = 0.0;

        for (int n = 0; n < segments; ++n){
            float px = x + cx;
            float py = y + cy;

            float t = x;
            x = c * x - s * y;
            y = c * y + s * t;

            kinc_g2_fill_triangle(px, py, x + cx, y + cy, cx, cy);
        }
}

void kinc_g2_ext_drawInnerLine(float x1, float y1, float  x2, float y2, float strength) {
    int side = y2 > y1 ? 1 : 0;
    if (y2 == y1)
        side = x2 - x1 > 0 ? 1 : 0;

    kinc_vector2_t vec = { 0 };
    if (y2 == y1) {
        vec.y = -1;
    }
    else {
        vec.x = 1;
        vec.y = -(x2 - x1) / (y2 - y1);
    }

    vec2_set_length(&vec, strength);
    kinc_vector2_t p1 = { 0 };
    p1.x = (x1 + side * vec.x);
    p1.y = (y1 + side * vec.y);
    kinc_vector2_t p2 = { 0 };
    p2.x = x2 + side * vec.x;
    p2.y = y2 + side * vec.y;
    kinc_vector2_t p3 = { 0 };
    vec2_sub(&p1, &vec, &p3);
    kinc_vector2_t p4 = { 0 };
    vec2_sub(&p2, &vec, &p4);
    kinc_g2_fill_triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    kinc_g2_fill_triangle(p3.x, p3.y, p2.x, p2.y, p4.x, p4.y);
}

void kinc_g2_ext_draw_polygon(float x ,float y,kinc_vector2_t* vertices ,size_t v_len, float strength ) {
    kinc_vector2_t v0 = vertices[0];
    kinc_vector2_t v1 = {0};
    v1.x = v0.x;
    v1.y = v0.y;
    int i = 1;
    while (i < v_len) {
        kinc_vector2_t v2 = vertices[i];
        kinc_g2_draw_line(v1.x + x, v1.y + y, v2.x + x, v2.y + y, strength);
        v1.x = v2.x;
        v1.y = v2.y;
        ++i;
    }
    kinc_g2_draw_line(v1.x + x, v1.y + y, v0.x + x, v0.y + y, strength);
}

void kinc_g2_ext_fill_polygon(float x, float y, kinc_vector2_t* vertices, size_t v_len){

    if (v_len < 3)
        return;
    kinc_vector2_t v0 = vertices[0];
    kinc_vector2_t v1 = vertices[1];

    int i = 2;
    while (i < v_len) {
        kinc_vector2_t v2 = vertices[i];
        kinc_g2_fill_triangle(v0.x + x, v0.y + y, v1.x + x, v1.y + y, v2.x + x, v2.y + y);
        v1.x = v2.x;
        v1.y = v2.y;
        ++i;
    }
}

static void calculateCubicBezierPoint(float t,float*  x,float* y,float* out){
        assert(out != NULL);
        float u  = 1.0f - t;
        float tt  = t * t;
        float uu  = u * u;
        float uuu  = uu * u;
        float ttt  = tt * t;

        // first term
        out[0] = uuu * x[0];
        out[1] = uuu * y[0];

        // second term
        out[0] += 3.0f * uu * t * x[1];
        out[1] += 3.0f * uu * t * y[1];

        // third term
        out[0] += 3.0f * u * tt * x[2];
        out[1] += 3.0f * u * tt * y[2];

        // fourth term
        out[0] += ttt * x[3];
        out[1] += ttt * y[3];
}

/**
     Draws a cubic bezier using 4 pairs of points. If the x and y arrays have a length bigger then 4, the additional
     points will be ignored. With a length smaller of 4 a error will occur, there is no check for this.
     You can construct the curves visually in Inkscape with a path using default nodes.
     Provide x and y in the following order: startPoint, controlPoint1, controlPoint2, endPoint
     Reference: http://devmag.org.za/2011/04/05/bzier-curves-a-tutorial/
*/
void kinc_g2_ext_draw_cubicBezier(float* x ,float* y , int segments,float strength){
    if (segments <= 20)
        segments = 20;
    if (strength <= 0)
        strength = 1;
    float t;

    float q0[2] = { 0 };
    calculateCubicBezierPoint(0, x, y,q0);
    float q1[2] = {0};

    for (int i = 1; i < (segments + 1);++i) {
        t = (float)i / segments;
        calculateCubicBezierPoint(t, x, y,q1);
        kinc_g2_draw_line(q0[0], q0[1], q1[0], q1[1], strength);
        q0[0] = q1[0];
        q0[1] = q1[1];
    }
}

/**
    Draws multiple cubic beziers joined by the end point. The minimum size is 4 pairs of points (a single curve).
 */
void kinc_g2_ext_draw_cubicBezierPath(float* x ,float* y ,int point_count,int segments, float strength){
        if (segments <= 0)
            segments = 20;
        if (strength <= 0)
            strength = 1;
        int i = 0;
        float t = 0;
        float q0[2] = {0};
        float q1[2] = {0};

        while (i < point_count - 3) {
            if (i == 0) {
                float tempX[4] = { x[i], x[i + 1], x[i + 2], x[i + 3] };
                float tempY[4] = { y[i], y[i + 1], y[i + 2], y[i + 3] };
                calculateCubicBezierPoint(t, tempX, tempY, q0);
            }


            for (int j = 1; j < (segments + 1);++j) {
                t = (float)j / segments;
                float tempX[4] = { x[i], x[i + 1], x[i + 2], x[i + 3] };
                float tempY[4] = { y[i], y[i + 1], y[i + 2], y[i + 3] };
                calculateCubicBezierPoint(t,tempX,tempY,q1);
                kinc_g2_draw_line(q0[0], q0[1], q1[0], q1[1], strength);
                q0[0] = q1[0];
                q0[1] = q1[1];
            }

            i += 3;
        }
}
