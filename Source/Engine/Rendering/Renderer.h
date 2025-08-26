/**
 * Renderer.h - Abstract Rendering Interface
 * 
 * Defines the abstract renderer contract. Concrete renderers (e.g., OpenGL-based)
 * select and compile their vertex/fragment shaders during initialization and
 * expose a common API for frame control and drawing meshes.
 */

#pragma once
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include "../Math/Math.h"
#include "../Math/Camera.h"

namespace Engine {

class Mesh;
class Shader;

/**
 * Abstract Renderer Interface
 */
class Renderer {
public:
    virtual ~Renderer() = default;

    // Initialization and teardown
    virtual bool initialize(int width, int height) = 0;
    virtual void cleanup() = 0;

    // Frame control
    virtual void beginFrame() = 0;
    virtual void endFrame(GLFWwindow* window) = 0;

    // Global configuration
    virtual void setViewport(int width, int height) = 0;
    virtual void setClearColor(float r, float g, float b, float a = 1.0f) = 0;

    // Rendering primitives
    virtual void renderMesh(const Mesh& mesh,
                            const Mat4& modelMatrix,
                            const Camera& camera,
                            const Vec3& color) const = 0;

    // Optional helpers
    virtual void renderCrosshair(const Camera& camera) const = 0;

    // Queries
    virtual float getAspectRatio() const = 0;
    virtual const Mat4& getProjectionMatrix() const = 0;
    
    // Get shader for custom rendering
    virtual Shader* getShader() const = 0;
};

} // namespace Engine