/**
 * Math.cpp - Implementation of Mathematical Utilities
 * 
 * Contains all implementations of vector operations, matrix transformations,
 * and mathematical functions used throughout the graphics engine.
 */

#include "Math.h"

namespace Engine {

// ===== VEC2 IMPLEMENTATIONS =====

Vec2 Vec2::operator+(const Vec2& other) const { 
    return Vec2(x + other.x, y + other.y); 
}

Vec2 Vec2::operator-(const Vec2& other) const { 
    return Vec2(x - other.x, y - other.y); 
}

Vec2 Vec2::operator*(float scalar) const { 
    return Vec2(x * scalar, y * scalar); 
}

float Vec2::length() const { 
    return sqrt(x * x + y * y); 
}

Vec2 Vec2::normalize() const { 
    float len = length(); 
    return len > 0 ? Vec2(x / len, y / len) : Vec2(0, 0); 
}

float Vec2::dot(const Vec2& other) const { 
    return x * other.x + y * other.y; 
}

// ===== VEC3 IMPLEMENTATIONS =====

Vec3 Vec3::operator+(const Vec3& other) const { 
    return Vec3(x + other.x, y + other.y, z + other.z); 
}

Vec3 Vec3::operator-(const Vec3& other) const { 
    return Vec3(x - other.x, y - other.y, z - other.z); 
}

Vec3 Vec3::operator*(float scalar) const { 
    return Vec3(x * scalar, y * scalar, z * scalar); 
}

float Vec3::length() const { 
    return sqrt(x * x + y * y + z * z); 
}

Vec3 Vec3::normalize() const { 
    float len = length(); 
    return len > 0 ? Vec3(x / len, y / len, z / len) : Vec3(0, 0, 0); 
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(y * other.z - z * other.y, 
               z * other.x - x * other.z, 
               x * other.y - y * other.x);
}

float Vec3::dot(const Vec3& other) const { 
    return x * other.x + y * other.y + z * other.z; 
}

Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

// ===== GLOBAL VEC2 FUNCTIONS =====

Vec2 normalize(const Vec2& v) {
    float len = sqrt(v.x * v.x + v.y * v.y);
    return len > 0 ? Vec2(v.x / len, v.y / len) : Vec2(0, 0);
}

// ===== GLOBAL VEC3 FUNCTIONS =====

Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y, 
               a.z * b.x - a.x * b.z, 
               a.x * b.y - a.y * b.x);
}

Vec3 normalize(const Vec3& v) {
    float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return len > 0 ? Vec3(v.x / len, v.y / len, v.z / len) : Vec3(0, 0, 0);
}

// ===== VEC4 IMPLEMENTATIONS =====

Vec4 Vec4::operator+(const Vec4& other) const { 
    return Vec4(x + other.x, y + other.y, z + other.z, w + other.w); 
}

Vec4 Vec4::operator-(const Vec4& other) const { 
    return Vec4(x - other.x, y - other.y, z - other.z, w - other.w); 
}

Vec4 Vec4::operator*(float scalar) const { 
    return Vec4(x * scalar, y * scalar, z * scalar, w * scalar); 
}

// ===== MAT3 IMPLEMENTATIONS =====

Mat3::Mat3() {
    for (int i = 0; i < 9; i++) m[i] = 0.0f;
    m[0] = m[4] = m[8] = 1.0f; // Identity matrix
}

Mat3::Mat3(float m00, float m01, float m02,
           float m10, float m11, float m12,
           float m20, float m21, float m22) {
    m[0] = m00; m[3] = m01; m[6] = m02;
    m[1] = m10; m[4] = m11; m[7] = m12;
    m[2] = m20; m[5] = m21; m[8] = m22;
}

// ===== MAT4 IMPLEMENTATIONS =====

Mat4::Mat4() {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f; // Identity matrix
}

Mat4::Mat4(float diagonal) {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = diagonal;
}

// ===== MATRIX OPERATIONS =====

Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i * 4 + j] += a.m[i * 4 + k] * b.m[k * 4 + j];
            }
        }
    }
    return result;
}

// Matrix-matrix multiplication
Mat4 operator*(const Mat4& a, const Mat4& b) {
    return multiply(a, b);  // Use existing multiply function
}

// Matrix-vector multiplication
Vec4 operator*(const Mat4& matrix, const Vec4& vector) {
    return Vec4(
        matrix.m[0] * vector.x + matrix.m[4] * vector.y + matrix.m[8] * vector.z + matrix.m[12] * vector.w,
        matrix.m[1] * vector.x + matrix.m[5] * vector.y + matrix.m[9] * vector.z + matrix.m[13] * vector.w,
        matrix.m[2] * vector.x + matrix.m[6] * vector.y + matrix.m[10] * vector.z + matrix.m[14] * vector.w,
        matrix.m[3] * vector.x + matrix.m[7] * vector.y + matrix.m[11] * vector.z + matrix.m[15] * vector.w
    );
}

// ===== TRANSFORMATION MATRICES =====

Mat4 perspective(float fovy, float aspect, float near, float far) {
    Mat4 result(0.0f);
    float tanHalfFovy = tan(fovy * 0.5f);
    
    result.m[0] = 1.0f / (aspect * tanHalfFovy);
    result.m[5] = 1.0f / tanHalfFovy;
    result.m[10] = -(far + near) / (far - near);
    result.m[11] = -1.0f;
    result.m[14] = -(2.0f * far * near) / (far - near);
    
    return result;
}

Mat4 orthographic(float left, float right, float bottom, float top, float near, float far) {
    Mat4 result;
    
    result.m[0] = 2.0f / (right - left);
    result.m[5] = 2.0f / (top - bottom);
    result.m[10] = -2.0f / (far - near);
    
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(far + near) / (far - near);
    
    return result;
}

Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye).normalize();
    Vec3 s = f.cross(up).normalize();
    Vec3 u = s.cross(f);
    
    Mat4 result;
    result.m[0] = s.x;   result.m[4] = s.y;   result.m[8] = s.z;
    result.m[1] = u.x;   result.m[5] = u.y;   result.m[9] = u.z;
    result.m[2] = -f.x;  result.m[6] = -f.y;  result.m[10] = -f.z;
    
    result.m[12] = -s.dot(eye);
    result.m[13] = -u.dot(eye);
    result.m[14] = f.dot(eye);
    
    return result;
}

// ===== MAT3 OPERATIONS =====

Mat3 transpose(const Mat3& matrix) {
    Mat3 result;
    result.m[0] = matrix.m[0]; result.m[3] = matrix.m[1]; result.m[6] = matrix.m[2];
    result.m[1] = matrix.m[3]; result.m[4] = matrix.m[4]; result.m[7] = matrix.m[5];
    result.m[2] = matrix.m[6]; result.m[5] = matrix.m[7]; result.m[8] = matrix.m[8];
    return result;
}

Mat3 inverse(const Mat3& matrix) {
    // For now, return identity matrix (proper inverse calculation can be added later)
    return Mat3();
}

// ===== TRANSFORMATION FUNCTIONS =====

Mat4 translate(const Mat4& matrix, const Vec3& v) {
    Mat4 result = matrix;
    result.m[12] += v.x;
    result.m[13] += v.y;
    result.m[14] += v.z;
    return result;
}

Mat4 scale(const Vec3& v) {
    Mat4 result;
    result.m[0] = v.x;
    result.m[5] = v.y;
    result.m[10] = v.z;
    return result;
}

Mat4 rotateX(float angle) {
    Mat4 result;
    float c = cos(angle);
    float s = sin(angle);
    
    result.m[5] = c;
    result.m[6] = -s;
    result.m[9] = s;
    result.m[10] = c;
    
    return result;
}

Mat4 rotateY(float angle) {
    Mat4 result;
    float c = cos(angle);
    float s = sin(angle);
    
    result.m[0] = c;
    result.m[2] = s;
    result.m[8] = -s;
    result.m[10] = c;
    
    return result;
}

Mat4 rotateZ(float angle) {
    Mat4 result;
    float c = cos(angle);
    float s = sin(angle);
    
    result.m[0] = c;
    result.m[1] = -s;
    result.m[4] = s;
    result.m[5] = c;
    
    return result;
}

} // namespace Engine