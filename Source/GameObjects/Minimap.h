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
#include "SimpleChunkTerrainGround.h"
#include <memory>
#include <vector>

namespace Engine {

class Scene;
class Renderer;
class Arrow;

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
    
    // Orthographic projection scope (world space units)
    float orthoLeft;    // Left boundary of captured area
    float orthoRight;   // Right boundary of captured area
    float orthoBottom;  // Bottom boundary of captured area
    float orthoTop;     // Top boundary of captured area
    float orthoNear;    // Near clipping plane
    float orthoFar;     // Far clipping plane
    
    // Rendering
    unsigned int framebuffer;
    unsigned int textureColorBuffer;
    unsigned int renderbuffer;
    
    // Shaders
    std::unique_ptr<Shader> minimapShader;      // For rendering minimap texture
    std::unique_ptr<Shader> orthographicShader; // For orthographic rendering
    std::unique_ptr<Shader> arrowShader;        // For rendering arrow with color
    
    // Camera for orthographic view
    Camera orthographicCamera;
    
    // GPU-based rendering: Store scene objects for individual rendering
    std::vector<const GameObject*> sceneObjects;
    
    // Caching and state management
    bool sceneObjectsDirty;
    int lastUpdateFrame;    
    Vec3 lastPlayerPosition;
    
    // Scene reference for gathering objects
    Scene* scene;
    
    // Player tracking
    Vec3 playerPosition;
    float playerYaw; // Store player direction for arrow rotation
    Vec3 playerForwardDirection; // Store player's forward direction vector
    
    // World offset for minimap movement
    Vec3 worldOffset; // Accumulated world offset for minimap movement
    
    // Entity system
    class Ground* groundReference;
    
    // Terrain system
    class SimpleChunkTerrainGround* terrainReference;
    
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
    void setPlayerPosition(const Vec3& position);
    
    // Entity system
    void setGroundReference(class Ground* ground) { groundReference = ground; }
    
    // Player direction arrow (separate object - non-owning pointer + owning pointer)
    Arrow* playerArrow;
    std::unique_ptr<Arrow> playerArrowPtr;  // Owns the arrow object
    void setPlayerDirection(const Vec3& direction);
    void setPlayerDirectionFromYaw(float yawDegrees);
    
    // Configurable scope
    float configurableScopeSize;
    void setScopeSize(float size) { configurableScopeSize = size; }
    float getScopeSize() const { return configurableScopeSize; }
    
    // Force update when needed
    void forceUpdate() { sceneObjectsDirty = true; }
    
    // Minimap dimension configuration
    void setMinimapDimensions(int width, int height);
    void setOrthographicScope(float left, float right, float bottom, float top, float near = 0.1f, float far = 100.0f);
    
    // Getters for current settings
    int getMinimapWidth() const { return minimapWidth; }
    int getMinimapHeight() const { return minimapHeight; }
    float getMinimapSize() const { return minimapSize; }
    void getOrthographicScope(float& left, float& right, float& bottom, float& top, float& near, float& far) const;
    
    // Orthographic camera control
    void updateOrthographicCamera(const Vec3& playerPosition);
    
private:
    // Helper methods
    bool initializeFramebuffer();
    bool initializeShaders();
    bool updateSceneObjects();  // GPU-based: Update list of objects to render
    void renderSceneToTexture();
    void renderMinimapTexture();
    void renderArrowOverlay();  // Render arrow as 2D overlay on minimap
    void renderCenterIndicator(); // Render simple center indicator
    void renderPlayerPositionIndicator(); // Render player position dot on minimap
    void setupMesh(); // Override from GameObject
    
    // Utility
    void cleanupFramebuffer();
    bool isValid() const { return isInitialized && isFramebufferInitialized; }
    
    // Safety check for coordinate validation
    bool isEntityInMinimapScope(const GameObject* entity) const;
};

} // namespace Engine
