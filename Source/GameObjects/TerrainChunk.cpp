/**
 * TerrainChunk.cpp - Terrain Chunk Implementation
 * 
 * Implements terrain chunks with actual height data and proper mesh generation.
 */

#include "TerrainChunk.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/BasicRenderer.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Engine {

TerrainChunk::TerrainChunk(const std::string& name, const Vec2& position, int size, float cubeSize)
    : Chunk(name, position, size, cubeSize), maxHeight(0), meshGenerated(false) {
    
    // Initialize terrain data
    terrainBlocks.resize(size * size * 32); // 32 height levels (reduced for stability)
    heightMap.resize(size * size);
    
    // Set default terrain color
    terrainColor = Vec3(0.4f, 0.3f, 0.2f); // Brown
    
    // Position the chunk at its world position
    Vec2 chunkPos = getChunkPosition();
    setPosition(Vec3(chunkPos.x * static_cast<float>(size), 0.0f, chunkPos.y * static_cast<float>(size)));
    
    // TerrainChunk created successfully
}

void TerrainChunk::setTerrainData(const std::vector<TerrainBlockType>& blocks) {
    if (blocks.size() != terrainBlocks.size()) {
                  << ", got " << blocks.size() << std::endl;
        return;
    }
    
    // Terrain data set successfully
    
    terrainBlocks = blocks;
    calculateMaxHeight();
    generateTerrainMesh();
}

void TerrainChunk::setHeightMap(const std::vector<float>& heights) {
    if (heights.size() != heightMap.size()) {
        return;
    }
    
    heightMap = heights;
    calculateMaxHeight();
    generateTerrainMesh();
}

void TerrainChunk::setTerrainColor(const Vec3& color) {
    terrainColor = color;
    setColor(color); // Update base chunk color
}

void TerrainChunk::calculateMaxHeight() {
    maxHeight = 0;
    
    // Find the highest non-air block
    for (int i = 0; i < static_cast<int>(terrainBlocks.size()); i++) {
        if (terrainBlocks[i] != TerrainBlockType::Air) {
            int y = i / (16 * 16); // Calculate Y from index
            maxHeight = std::max(maxHeight, y);
        }
    }
    
    // TerrainChunk max height calculated
}

void TerrainChunk::generateTerrainMesh() {
    terrainVertices.clear();
    terrainIndices.clear();
    terrainNormals.clear();
    
    int chunkSize = 16;
    
    // Generate vertices for terrain surface
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            // Find height at this position
            float height = 0.0f;
            for (int y = 31; y >= 0; y--) { // Reduced height range
                int blockIndex = y * chunkSize * chunkSize + z * chunkSize + x;
                if (blockIndex < static_cast<int>(terrainBlocks.size()) && 
                    terrainBlocks[blockIndex] != TerrainBlockType::Air) {
                    height = static_cast<float>(y);
                    break;
                }
            }
            
            // Calculate local position within chunk
            float localX = static_cast<float>(x);
            float localZ = static_cast<float>(z);
            float localY = height;
            

            
            // Add vertex (in local chunk coordinates)
            terrainVertices.push_back(localX);
            terrainVertices.push_back(localY);
            terrainVertices.push_back(localZ);
            
            // Add normal (simplified - pointing up)
            terrainNormals.push_back(0.0f);
            terrainNormals.push_back(1.0f);
            terrainNormals.push_back(0.0f);
        }
    }
    
    // Generate indices for terrain surface
    for (int x = 0; x < chunkSize - 1; x++) {
        for (int z = 0; z < chunkSize - 1; z++) {
            int baseIndex = x * chunkSize + z;
            
            // First triangle
            terrainIndices.push_back(baseIndex);
            terrainIndices.push_back(baseIndex + 1);
            terrainIndices.push_back(baseIndex + chunkSize);
            
            // Second triangle
            terrainIndices.push_back(baseIndex + 1);
            terrainIndices.push_back(baseIndex + chunkSize + 1);
            terrainIndices.push_back(baseIndex + chunkSize);
        }
    }
    

    
    meshGenerated = true;
        // Terrain mesh generated successfully
    
    // Debug: Check if we have any non-zero heights
    int nonZeroHeights = 0;
    float minHeight = 1000.0f;
    float maxHeight = -1000.0f;
    for (size_t i = 0; i < terrainVertices.size(); i += 3) {
        float height = terrainVertices[i + 1]; // Y coordinate
        if (height > 0.1f) {
            nonZeroHeights++;
            minHeight = std::min(minHeight, height);
            maxHeight = std::max(maxHeight, height);
        }
    }
    // Height statistics calculated
}

void TerrainChunk::updateMesh() {
    if (meshGenerated) {
        // Update the mesh with new terrain data
        generateTerrainMesh();
    }
}

void TerrainChunk::setupMesh() {
    // Call parent setupMesh first
    Chunk::setupMesh();
    
    // Generate terrain mesh if we have terrain data
    if (!terrainBlocks.empty()) {
        generateTerrainMesh();
    }
}

void TerrainChunk::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid()) {
        return;
    }
    
    // TerrainChunk rendering
    
    // Use our own terrain mesh if it's generated, otherwise fall back to parent
    if (meshGenerated && !terrainVertices.empty()) {
        // Create a temporary mesh from our terrain data
        auto terrainMesh = std::make_unique<Mesh>();
        if (terrainMesh->createMesh(terrainVertices, terrainIndices)) {
            Mat4 modelMatrix = getModelMatrix();
            
            // Try to use BasicRenderer's height-based coloring if available
            const BasicRenderer* basicRenderer = dynamic_cast<const BasicRenderer*>(&renderer);
            if (basicRenderer) {
                // Use height-based coloring for terrain
                basicRenderer->renderMesh(*terrainMesh, modelMatrix, camera, terrainColor, true);
            } else {
                // Fall back to regular rendering
                renderer.renderMesh(*terrainMesh, modelMatrix, camera, terrainColor);
            }
            
            // Terrain mesh rendered successfully
        } else {
            // Failed to create terrain mesh, falling back to parent
            Chunk::render(renderer, camera);
        }
    } else {
        // No terrain mesh generated, using parent chunk rendering
        Chunk::render(renderer, camera);
    }
}

Vec3 TerrainChunk::getBlockColor(TerrainBlockType blockType) const {
    switch (blockType) {
        case TerrainBlockType::Grass:
            return Vec3(0.2f, 0.8f, 0.2f); // Green
        case TerrainBlockType::Dirt:
            return Vec3(0.6f, 0.4f, 0.2f); // Brown
        case TerrainBlockType::Stone:
            return Vec3(0.5f, 0.5f, 0.5f); // Gray
        case TerrainBlockType::Bedrock:
            return Vec3(0.2f, 0.2f, 0.2f); // Dark gray
        case TerrainBlockType::Water:
            return Vec3(0.0f, 0.5f, 1.0f); // Blue
        case TerrainBlockType::Sand:
            return Vec3(0.9f, 0.9f, 0.6f); // Sand color
        case TerrainBlockType::Air:
        default:
            return Vec3(0.0f, 0.0f, 0.0f); // Black (shouldn't be rendered)
    }
}

} // namespace Engine
