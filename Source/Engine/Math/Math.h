/**
 * Math.h - Mathematical Utilities for 3D Graphics Engine
 * 
 * OVERVIEW:
 * Core mathematical structures and operations for 3D computer graphics.
 * Provides vectors, matrices, and transformation functions used throughout the engine.
 * 
 * MATHEMATICAL CONCEPTS:
 * - 3D vectors for positions, directions, and calculations
 * - 4x4 matrices for transformations (translation, rotation, projection)
 * - Homogeneous coordinates for unified transformation pipeline
 * - Standard graphics transformations (perspective, lookAt, etc.)
 */

#pragma once
#include <cmath>

namespace Engine {

/**
 * 2D Vector Structure for Texture Coordinates and Screen Space
 * 
 * Used primarily for texture coordinate mapping (UV coordinates).
 * Also useful for screen space calculations and 2D operations.
 */
struct Vec2 {
    float x, y;
    
    // Constructors
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    
    // Vector operations
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(float scalar) const;
    
    // Mathematical operations
    float length() const;
    Vec2 normalize() const;
    float dot(const Vec2& other) const;
};

/**
 * 3D Vector Structure for Spatial Mathematics
 * 
 * MATHEMATICAL FOUNDATION:
 * Vectors in 3D space represent both position (points) and direction (vectors).
 * They form the basis of all 3D transformations and calculations.
 * 
 * COORDINATE SYSTEM:
 * We use a right-handed coordinate system where:
 * - X-axis points right
 * - Y-axis points up  
 * - Z-axis points toward the viewer (out of screen)
 */
struct Vec3 {
    float x, y, z;
    
    // Constructors
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    // Vector operations
    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    
    // Mathematical operations
    float length() const;
    Vec3 normalize() const;
    Vec3 cross(const Vec3& other) const;
    float dot(const Vec3& other) const;
};

/**
 * 3x3 Matrix Structure for Normal Transformations
 * 
 * Used primarily for transforming normal vectors in lighting calculations.
 * Represents the upper-left 3x3 portion of a 4x4 transformation matrix.
 */
struct Mat3 {
    float m[9];
    
    // Constructors
    Mat3();                    // Identity matrix
    Mat3(float m00, float m01, float m02,
         float m10, float m11, float m12,
         float m20, float m21, float m22);
    
    // Data access
    float* data() { return m; }
    const float* data() const { return m; }
};

/**
 * 4x4 Matrix Structure for 3D Transformations
 * 
 * MATHEMATICAL FOUNDATION:
 * A 4x4 matrix in computer graphics is used for homogeneous coordinates, allowing us to
 * represent translation, rotation, scaling, and projection in a single matrix operation.
 * 
 * MATRIX LAYOUT (Column-Major, OpenGL Style):
 * | m[0]  m[4]  m[8]   m[12] |   | Xx  Yx  Zx  Tx |
 * | m[1]  m[5]  m[9]   m[13] |   | Xy  Yy  Zy  Ty |
 * | m[2]  m[6]  m[10]  m[14] | = | Xz  Yz  Zz  Tz |
 * | m[3]  m[7]  m[11]  m[15] |   | 0   0   0   1  |
 */
struct Mat4 {
    float m[16];
    
    // Constructors
    Mat4();                    // Identity matrix
    Mat4(float diagonal);      // Diagonal matrix
    
    // Data access
    float* data() { return m; }
    const float* data() const { return m; }
};

// Vector utility functions
Vec2 normalize(const Vec2& v);
Vec3 cross(const Vec3& a, const Vec3& b);
Vec3 normalize(const Vec3& v);

// Matrix operations
Mat4 multiply(const Mat4& a, const Mat4& b);
Mat3 transpose(const Mat3& matrix);
Mat3 inverse(const Mat3& matrix);

// Transformation matrices
Mat4 perspective(float fovy, float aspect, float near, float far);
Mat4 orthographic(float left, float right, float bottom, float top, float near, float far);
Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
Mat4 translate(const Mat4& matrix, const Vec3& v);
Mat4 scale(const Vec3& v);
Mat4 rotateX(float angle);
Mat4 rotateY(float angle);
Mat4 rotateZ(float angle);

} // namespace Engine