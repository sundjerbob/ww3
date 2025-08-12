/**
 * Minimap.h - 2D Minimap GameObject
 * 
 * OVERVIEW:
 * Renders a 2D minimap texture in the top-left corner of the screen.
 * Uses orthographic projection to create a bird's-eye view of the scene.
 * 
 * FEATURES:
 * - 2D texture rendering in screen space
 * - Orthographic camera for top-down view
 * - Scene mesh aggregation
 * - Render-to-texture functionality
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Shader.h"
#include "../Engine/Math/Camera.h"
#include <memory>
#include <vector>

namespace Engine {

class Scene;
class Renderer;

/**
 * Minimap - 2D Minimap GameObject
 * 
 * Renders a bird's-eye view of the scene as a 2D texture overlay.
 * Aggregates all scene objects (except crosshair) into a single mesh
 * and renders them from an orthographic camera perspective.
 */
class Minimap : public GameObject {
private:
    // Minimap properties
    int minimapWidth;
    int minimapHeight;
    float minimapSize;  // Size on screen (0.0 to 1.0)
    
    // Rendering
    unsigned int framebuffer;
    unsigned int textureColorBuffer;
    unsigned int renderbuffer;
    
    // Shaders
    std::unique_ptr<Shader> minimapShader;      // For rendering minimap texture
    std::unique_ptr<Shader> orthographicShader; // For orthographic rendering
    
    // Camera for orthographic view
    Camera orthographicCamera;
    
    // Aggregated mesh from scene objects
    std::unique_ptr<Mesh> aggregatedMesh;
    
    // Scene reference for gathering objects
    Scene* scene;
    
    // State
    bool isFramebufferInitialized;
    bool isTextureValid;

public:
    // Constructor/Destructor
    Minimap(const std::string& name = "Minimap", float size = 0.2f);
    virtual ~Minimap();
    
    // Lifecycle methods
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    virtual void cleanup() override;
    
    // Setup
    void setScene(Scene* sceneRef) { scene = sceneRef; }
    void setMinimapSize(float size) { minimapSize = size; }
    
    // Orthographic camera control
    void updateOrthographicCamera(const Vec3& playerPosition);
    
private:
    // Helper methods
    bool initializeFramebuffer();
    bool initializeShaders();
    bool aggregateSceneMeshes();
    void renderSceneToTexture();
    void renderMinimapTexture();
    void setupMesh(); // Override from GameObject
    
    // Utility
    void cleanupFramebuffer();
    bool isValid() const { return isInitialized && isFramebufferInitialized; }
};

} // namespace Engine
