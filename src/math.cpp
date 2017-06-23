#include <math.h>

const float PI = 3.14159265358979323846f;

float square(float x) {
    return x * x;
}

float to_radians(float degrees) {
    return degrees * PI / 180.0f;
}

float to_degrees(float radians) {
    return radians * 180.0f / PI;
}

float lerp(float from, float step, float to) {
    return ((1.0f - step) * from) + (step * to);
}

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

Vector2 make_vector2(float x, float y) {
    Vector2 vector2;

    vector2.x = x;
    vector2.y = y;

    return vector2;
}

Vector2 make_vector2() {
    return make_vector2(0.0f, 0.0f);
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

Vector2 operator *(Vector2 a, float b) {
    return make_vector2(a.x * b, a.y * b);
}

Vector2& operator *=(Vector2& a, float b) {
    return a = a * b;
}

Vector2 operator *(float a, Vector2 b) {
    return b * a;
}

float get_length_squared(Vector2 a) {
    return square(a.x) + square(a.y);
}

float get_length(Vector2 a) {
    return sqrtf(get_length_squared(a));
}

Vector2 normalize(Vector2 a) {
    float length = get_length(a);
    if (!length) {
        return make_vector2();
    }

    return make_vector2(a.x / length, a.y / length);
}

Vector2 lerp(Vector2 from, float step, Vector2 to) {
    return ((1.0f - step) * from) + (step * to);
}

Vector2 move_towards(Vector2 from, float step, Vector2 to) {
    return from + (normalize(to - from) * step);
}

Vector2 get_direction(float angle) {
    float x = -sinf(to_radians(angle));
    float y =  cosf(to_radians(angle));

    return make_vector2(x, y);
}

float get_angle(Vector2 direction) {
    return to_degrees(atan2f(direction.y, direction.x)) - 90.0f;
}

struct Matrix4 {
    float _11 = 0.0f;
    float _12 = 0.0f;
    float _13 = 0.0f;
    float _14 = 0.0f;
    float _21 = 0.0f;
    float _22 = 0.0f;
    float _23 = 0.0f;
    float _24 = 0.0f;
    float _31 = 0.0f;
    float _32 = 0.0f;
    float _33 = 0.0f;
    float _34 = 0.0f;
    float _41 = 0.0f;
    float _42 = 0.0f;
    float _43 = 0.0f;
    float _44 = 0.0f;
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
    float x = (matrix._11 * vector.x) + (matrix._21 * vector.y) + matrix._31 + matrix._41;
    float y = (matrix._12 * vector.x) + (matrix._22 * vector.y) + matrix._32 + matrix._42;

    return make_vector2(x, y);
}

Matrix4 make_orthographic_matrix(float left, float right, float top, float bottom, float far_plane = 1.0f, float near_plane = -1.0f) {
    Matrix4 matrix = make_identity_matrix();

    matrix._11 =  2.0f / (right - left);
    matrix._22 =  2.0f / (top - bottom);
    matrix._33 = -2.0f / (far_plane - near_plane);

    matrix._41 = -(right + left) / (right - left);
    matrix._42 = -(top + bottom) / (top - bottom);
    matrix._43 = -(far_plane + near_plane) / (far_plane - near_plane);

    return matrix;
}

Matrix4 make_transform_matrix(Vector2 position, float orientation = 0.0f, float scale = 1.0f) {
    Matrix4 translation = make_identity_matrix();
    Matrix4 rotation    = make_identity_matrix();
    Matrix4 scalar      = make_identity_matrix();

    translation._41 = position.x;
    translation._42 = position.y;

    float s = sinf(to_radians(orientation));
    float c = cosf(to_radians(orientation));

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

    float determinant = 
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