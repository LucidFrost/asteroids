#include <math.h>

const f32 PI = 3.14159265358979323846f;

f32 square(f32 x) {
    return x * x;
}

f32 to_radians(f32 degrees) {
    return degrees * PI / 180.0f;
}

f32 to_degrees(f32 radians) {
    return radians * 180.0f / PI;
}

f32 lerp(f32 from, f32 step, f32 to) {
    return ((1.0f - step) * from) + (step * to);
}

void seed_random() {
    srand((u32) time(null));
}

u32 get_random_u32() {
    u32 result = rand();
    return result;
}

u32 get_random_out_of(u32 count) {
    return get_random_u32() % count;
}

bool get_random_chance(u32 chance) {
    return get_random_out_of(chance) == chance - 1;
}

f32 get_random_unilateral() {
    return (f32) get_random_u32() / (f32) RAND_MAX;
}

f32 get_random_bilateral() {
    return (2.0f * get_random_unilateral()) - 1.0f;
}

f32 get_random_between(f32 min, f32 max) {
    return lerp(min, get_random_unilateral(), max);
}

i32 get_random_between(i32 min, i32 max) {
    return min + ((i32) get_random_u32() % ((max + 1) - min));
}

struct Vector2 {
    f32 x = 0.0f;
    f32 y = 0.0f;
};

Vector2 make_vector2(f32 x, f32 y) {
    Vector2 vector2;

    vector2.x = x;
    vector2.y = y;

    return vector2;
}

Vector2 operator +(Vector2 a, Vector2 b) {
    return make_vector2(a.x + b.x, a.y + b.y);
}

Vector2& operator +=(Vector2& a, Vector2 b) {
    return a = a + b;
}

Vector2 operator -(Vector2 a, Vector2 b) {
    return make_vector2(a.x - b.x, a.y - b.y);
}

Vector2& operator -=(Vector2& a, Vector2 b) {
    return a = a - b;
}

Vector2 operator *(Vector2 a, f32 b) {
    return make_vector2(a.x * b, a.y * b);
}

Vector2& operator *=(Vector2& a, f32 b) {
    return a = a * b;
}

Vector2 operator *(f32 a, Vector2 b) {
    return b * a;
}

Vector2 operator /(Vector2 a, f32 b) {
    return make_vector2(a.x / b, a.y / b);
}

Vector2& operator /=(Vector2& a, f32 b) {
    return a = a / b;
}

Vector2 operator /(f32 a, Vector2 b) {
    return b / a;
}

f32 get_length_squared(Vector2 a) {
    return square(a.x) + square(a.y);
}

f32 get_length(Vector2 a) {
    return sqrtf(get_length_squared(a));
}

Vector2 normalize(Vector2 a) {
    f32 length = get_length(a);
    if (!length) {
        return make_vector2(0.0f, 0.0f);
    }

    return make_vector2(a.x / length, a.y / length);
}

Vector2 lerp(Vector2 from, f32 step, Vector2 to) {
    return ((1.0f - step) * from) + (step * to);
}

Vector2 move_towards(Vector2 from, f32 step, Vector2 to) {
    return from + (normalize(to - from) * step);
}

Vector2 get_direction(f32 angle) {
    f32 x = -sinf(to_radians(angle));
    f32 y =  cosf(to_radians(angle));

    return make_vector2(x, y);
}

f32 get_angle(Vector2 direction) {
    return to_degrees(atan2f(direction.y, direction.x)) - 90.0f;
}

struct Rectangle2 {
    f32 x1 = 0.0f;
    f32 y1 = 0.0f;
    f32 x2 = 0.0f;
    f32 y2 = 0.0f;
};

Rectangle2 make_rectangle2(f32 x1, f32 y1, f32 x2, f32 y2) {
    Rectangle2 rectangle2;

    rectangle2.x1 = x1;
    rectangle2.y1 = y1;
    rectangle2.x2 = x2;
    rectangle2.y2 = y2;

    return rectangle2;
}

Rectangle2 make_rectangle2(Vector2 position, f32 width, f32 height, bool center = false) {
    if (center) {
        position.x -= width  / 2.0f;
        position.y -= height / 2.0f;
    }

    return make_rectangle2(position.x, position.y, position.x + width, position.y + height);
}

bool contains(Rectangle2 rectangle2, Vector2 position) {
    if (position.x < rectangle2.x1) return false;
    if (position.y < rectangle2.y1) return false;
    if (position.x > rectangle2.x2) return false;
    if (position.y > rectangle2.y2) return false;

    return true;
}

struct Circle {
    Vector2 position;
    f32 radius = 0.0f;
};

Circle make_circle(Vector2 position, f32 radius) {
    Circle circle;

    circle.position = position;
    circle.radius   = radius;

    return circle;
}

bool contains(Circle circle, Vector2 position) {
    return get_length(position - circle.position) <= circle.radius;
}

bool intersects(Circle circle_a, Circle circle_b) {
    return get_length(circle_b.position - circle_a.position) <= circle_a.radius + circle_b.radius;
}

struct Matrix4 {
    f32 _11 = 0.0f;
    f32 _12 = 0.0f;
    f32 _13 = 0.0f;
    f32 _14 = 0.0f;
    f32 _21 = 0.0f;
    f32 _22 = 0.0f;
    f32 _23 = 0.0f;
    f32 _24 = 0.0f;
    f32 _31 = 0.0f;
    f32 _32 = 0.0f;
    f32 _33 = 0.0f;
    f32 _34 = 0.0f;
    f32 _41 = 0.0f;
    f32 _42 = 0.0f;
    f32 _43 = 0.0f;
    f32 _44 = 0.0f;
};

Matrix4 make_identity_matrix() {
    Matrix4 matrix;

    matrix._11 = 1.0;
    matrix._22 = 1.0;
    matrix._33 = 1.0;
    matrix._44 = 1.0;

    return matrix;
}

Matrix4 operator *(Matrix4 a, Matrix4 b) {
    Matrix4 matrix;

    matrix._11 = (a._11 * b._11) + (a._21 * b._12) + (a._31 * b._13) + (a._41 * b._14);
    matrix._21 = (a._11 * b._21) + (a._21 * b._22) + (a._31 * b._23) + (a._41 * b._24);
    matrix._31 = (a._11 * b._31) + (a._21 * b._32) + (a._31 * b._33) + (a._41 * b._34);
    matrix._41 = (a._11 * b._41) + (a._21 * b._42) + (a._31 * b._43) + (a._41 * b._44);

    matrix._12 = (a._12 * b._11) + (a._22 * b._12) + (a._32 * b._13) + (a._42 * b._14);
    matrix._22 = (a._12 * b._21) + (a._22 * b._22) + (a._32 * b._23) + (a._42 * b._24);
    matrix._32 = (a._12 * b._31) + (a._22 * b._32) + (a._32 * b._33) + (a._42 * b._34);
    matrix._42 = (a._12 * b._41) + (a._22 * b._42) + (a._32 * b._43) + (a._42 * b._44);
    
    matrix._13 = (a._13 * b._11) + (a._23 * b._12) + (a._33 * b._13) + (a._43 * b._14);
    matrix._23 = (a._13 * b._21) + (a._23 * b._22) + (a._33 * b._23) + (a._43 * b._24);
    matrix._33 = (a._13 * b._31) + (a._23 * b._32) + (a._33 * b._33) + (a._43 * b._34);
    matrix._43 = (a._13 * b._41) + (a._23 * b._42) + (a._33 * b._43) + (a._43 * b._44);

    matrix._14 = (a._14 * b._11) + (a._24 * b._12) + (a._34 * b._13) + (a._44 * b._14);
    matrix._24 = (a._14 * b._21) + (a._24 * b._22) + (a._34 * b._23) + (a._44 * b._24);
    matrix._34 = (a._14 * b._31) + (a._24 * b._32) + (a._34 * b._33) + (a._44 * b._34);
    matrix._44 = (a._14 * b._41) + (a._24 * b._42) + (a._34 * b._43) + (a._44 * b._44);

    return matrix;
}

Vector2 operator *(Matrix4 matrix, Vector2 vector) {
    f32 x = (matrix._11 * vector.x) + (matrix._21 * vector.y) + matrix._31 + matrix._41;
    f32 y = (matrix._12 * vector.x) + (matrix._22 * vector.y) + matrix._32 + matrix._42;

    return make_vector2(x, y);
}

Matrix4 make_orthographic_matrix(f32 left, f32 right, f32 top, f32 bottom, f32 near_plane = -1.0f, f32 far_plane = 1.0f) {
    Matrix4 matrix = make_identity_matrix();

    matrix._11 =  2.0f / (right - left);
    matrix._22 =  2.0f / (top - bottom);
    matrix._33 = -2.0f / (far_plane - near_plane);

    matrix._41 = -(right + left) / (right - left);
    matrix._42 = -(top + bottom) / (top - bottom);
    matrix._43 = -(far_plane + near_plane) / (far_plane - near_plane);

    return matrix;
}

Matrix4 make_transform_matrix(Vector2 position, f32 orientation = 0.0f, f32 scale = 1.0f) {
    Matrix4 translation = make_identity_matrix();
    Matrix4 rotation    = make_identity_matrix();
    Matrix4 scalar      = make_identity_matrix();

    translation._41 = position.x;
    translation._42 = position.y;

    f32 s = sinf(to_radians(orientation));
    f32 c = cosf(to_radians(orientation));

    rotation._11 =  c;
    rotation._21 = -s;
    rotation._12 =  s;
    rotation._22 =  c;

    scalar._11 = scale;
    scalar._22 = scale;

    return translation * rotation * scalar;
}

// @note: This was lifted from the MESA implementation of the GLU library.
Matrix4 make_inverse_matrix(Matrix4 matrix) {
    Matrix4 inverse;

    inverse._11 = 
         matrix._22 * matrix._33 * matrix._44 - 
         matrix._22 * matrix._34 * matrix._43 - 
         matrix._32 * matrix._23 * matrix._44 + 
         matrix._32 * matrix._24 * matrix._43 +
         matrix._42 * matrix._23 * matrix._34 - 
         matrix._42 * matrix._24 * matrix._33;

    inverse._12 = 
        -matrix._12 * matrix._33 * matrix._44 + 
         matrix._12 * matrix._34 * matrix._43 + 
         matrix._32 * matrix._13 * matrix._44 - 
         matrix._32 * matrix._14 * matrix._43 - 
         matrix._42 * matrix._13 * matrix._34 + 
         matrix._42 * matrix._14 * matrix._33;

    inverse._13 =
         matrix._12 * matrix._23 * matrix._44 - 
         matrix._12 * matrix._24 * matrix._43 - 
         matrix._22 * matrix._13 * matrix._44 + 
         matrix._22 * matrix._14 * matrix._43 + 
         matrix._42 * matrix._13 * matrix._24 - 
         matrix._42 * matrix._14 * matrix._23;

    inverse._14 =
        -matrix._12 * matrix._23 * matrix._34 + 
         matrix._12 * matrix._24 * matrix._33 + 
         matrix._22 * matrix._13 * matrix._34 - 
         matrix._22 * matrix._14 * matrix._33 - 
         matrix._32 * matrix._13 * matrix._24 + 
         matrix._32 * matrix._14 * matrix._23;

    inverse._21 =
        -matrix._21 * matrix._33 * matrix._44 + 
         matrix._21 * matrix._34 * matrix._43 + 
         matrix._31 * matrix._23 * matrix._44 - 
         matrix._31 * matrix._24 * matrix._43 - 
         matrix._41 * matrix._23 * matrix._34 + 
         matrix._41 * matrix._24 * matrix._33;

    inverse._22 =
         matrix._11 * matrix._33 * matrix._44 - 
         matrix._11 * matrix._34 * matrix._43 - 
         matrix._31 * matrix._13 * matrix._44 + 
         matrix._31 * matrix._14 * matrix._43 + 
         matrix._41 * matrix._13 * matrix._34 - 
         matrix._41 * matrix._14 * matrix._33;

    inverse._23 =
        -matrix._11 * matrix._23 * matrix._44 + 
         matrix._11 * matrix._24 * matrix._43 + 
         matrix._21 * matrix._13 * matrix._44 - 
         matrix._21 * matrix._14 * matrix._43 - 
         matrix._41 * matrix._13 * matrix._24 + 
         matrix._41 * matrix._14 * matrix._23;

    inverse._24 =
         matrix._11 * matrix._23 * matrix._34 - 
         matrix._11 * matrix._24 * matrix._33 - 
         matrix._21 * matrix._13 * matrix._34 + 
         matrix._21 * matrix._14 * matrix._33 + 
         matrix._31 * matrix._13 * matrix._24 - 
         matrix._31 * matrix._14 * matrix._23;

    inverse._31 =
         matrix._21 * matrix._32 * matrix._44 - 
         matrix._21 * matrix._34 * matrix._42 - 
         matrix._31 * matrix._22 * matrix._44 + 
         matrix._31 * matrix._24 * matrix._42 + 
         matrix._41 * matrix._22 * matrix._34 - 
         matrix._41 * matrix._24 * matrix._32;

    inverse._32 =
        -matrix._11 * matrix._32 * matrix._44 + 
         matrix._11 * matrix._34 * matrix._42 + 
         matrix._31 * matrix._12 * matrix._44 - 
         matrix._31 * matrix._14 * matrix._42 - 
         matrix._41 * matrix._12 * matrix._34 + 
         matrix._41 * matrix._14 * matrix._32;

    inverse._33 =
         matrix._11 * matrix._22 * matrix._44 - 
         matrix._11 * matrix._24 * matrix._42 - 
         matrix._21 * matrix._12 * matrix._44 + 
         matrix._21 * matrix._14 * matrix._42 + 
         matrix._41 * matrix._12 * matrix._24 - 
         matrix._41 * matrix._14 * matrix._22;

    inverse._34 =
        -matrix._11 * matrix._22 * matrix._34 + 
         matrix._11 * matrix._24 * matrix._32 + 
         matrix._21 * matrix._12 * matrix._34 - 
         matrix._21 * matrix._14 * matrix._32 - 
         matrix._31 * matrix._12 * matrix._24 + 
         matrix._31 * matrix._14 * matrix._22;

    inverse._41 =
        -matrix._21 * matrix._32 * matrix._43 + 
         matrix._21 * matrix._33 * matrix._42 +
         matrix._31 * matrix._22 * matrix._43 - 
         matrix._31 * matrix._23 * matrix._42 - 
         matrix._41 * matrix._22 * matrix._33 + 
         matrix._41 * matrix._23 * matrix._32;

    inverse._42 =
         matrix._11 * matrix._32 * matrix._43 - 
         matrix._11 * matrix._33 * matrix._42 - 
         matrix._31 * matrix._12 * matrix._43 + 
         matrix._31 * matrix._13 * matrix._42 + 
         matrix._41 * matrix._12 * matrix._33 - 
         matrix._41 * matrix._13 * matrix._32;

    inverse._43 =
        -matrix._11 * matrix._22 * matrix._43 + 
         matrix._11 * matrix._23 * matrix._42 + 
         matrix._21 * matrix._12 * matrix._43 - 
         matrix._21 * matrix._13 * matrix._42 - 
         matrix._41 * matrix._12 * matrix._23 + 
         matrix._41 * matrix._13 * matrix._22;

    inverse._44 =
         matrix._11 * matrix._22 * matrix._33 - 
         matrix._11 * matrix._23 * matrix._32 - 
         matrix._21 * matrix._12 * matrix._33 + 
         matrix._21 * matrix._13 * matrix._32 + 
         matrix._31 * matrix._12 * matrix._23 - 
         matrix._31 * matrix._13 * matrix._22;

    f32 determinant = 
         matrix._11 * inverse._11 + 
         matrix._12 * inverse._21 + 
         matrix._13 * inverse._31 + 
         matrix._14 * inverse._41;

    assert(determinant != 0.0f);
    determinant = 1.0f / determinant;

    inverse._11 *= determinant;
    inverse._12 *= determinant;
    inverse._13 *= determinant;
    inverse._14 *= determinant;
    inverse._21 *= determinant;
    inverse._22 *= determinant;
    inverse._23 *= determinant;
    inverse._24 *= determinant;
    inverse._31 *= determinant;
    inverse._32 *= determinant;
    inverse._33 *= determinant;
    inverse._34 *= determinant;
    inverse._41 *= determinant;
    inverse._42 *= determinant;
    inverse._43 *= determinant;
    inverse._44 *= determinant;

    return inverse;
}

Vector2 unproject(i32 window_x, i32 window_y, u32 window_width, u32 window_height, Matrix4 projection) {
    f32 normalized_x = ((2.0f * window_x) / window_width) - 1.0f;
    f32 normalized_y = 1.0f - ((2.0f * window_y) / window_height);

    return make_inverse_matrix(projection) * make_vector2(normalized_x, normalized_y);
}