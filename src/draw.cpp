#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

void init_draw() {

}

void set_projection(Matrix4* projection) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&projection->_11);
}

void set_transform(Matrix4* transform) {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&transform->_11);
}

void draw_rectangle(float width, float height, float r, float g, float b, float a) {
    glBegin(GL_QUADS);
    glColor4f(r, g, b, a);

    float half_width  = width  / 2.0f;
    float half_height = height / 2.0f;

    glVertex2f(-half_width, -half_height);
    glVertex2f( half_width, -half_height);
    glVertex2f( half_width,  half_height);
    glVertex2f(-half_width,  half_height);

    glEnd();
}