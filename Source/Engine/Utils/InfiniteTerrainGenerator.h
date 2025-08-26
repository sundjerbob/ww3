/**
 * InfiniteTerrainGenerator.h - Infinite Procedural Terrain Generation
 * 
 * OVERVIEW:
 * Combines the good Perlin noise from CProceduralGame with infinite terrain generation.
 * Generates terrain chunks dynamically as the player moves, creating an infinite world.
 * 
 * FEATURES:
 * - Infinite terrain generation using Perlin noise
 * - Dynamic chunk loading/unloading based on player position
 * - Configurable terrain parameters
 * - Layered terrain (grass, dirt, stone, bedrock)
 * - Water level system
 * - Memory-efficient chunk management
 * - Seamless terrain transitions
 */

#pragma once
#include "../Math/Math.h"
#include "TerrainGenerator.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <functional>

namespace Engine {

// Chunk coordinate system
struct ChunkCoord {
    int x, z;
    
    ChunkCoord(int chunkX, int chunkZ) : x(chunkX), z(chunkZ) {}
    
    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
    
    bool operator<(const ChunkCoord& other) const {
        if (x != other.x) return x < other.x;
        return z < other.z;
    }
    
    // For unordered_map
    struct Hash {
        std::size_t operator()(const ChunkCoord& coord) const {
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.z) << 1);
        }
    };
};

// Chunk data structure
struct TerrainChunkData {
    std::vector<TerrainBlockType> blocks;
    bool isGenerated;
    bool isLoaded;
    float lastAccessTime;
    
    TerrainChunkData() : isGenerated(false), isLoaded(false), lastAccessTime(0.0f) {}
};

/**
 * InfiniteTerrainGenerator - Infinite terrain generation system
 * 
 * Manages infinite terrain generation using a chunk-based system.
 * Dynamically loads and unloads chunks based on player position.
 */
class InfiniteTerrainGenerator {
private:
    // Core terrain generator
    TerrainGenerator terrainGenerator;
    
    // Chunk management
    std::unordered_map<ChunkCoord, TerrainChunkData, ChunkCoord::Hash> chunks;
    std::queue<ChunkCoord> chunkLoadQueue;
    std::queue<ChunkCoord> chunkUnloadQueue;
    
    // Configuration
    int chunkSize;
    int chunkHeight;
    int renderDistance;      // How many chunks to keep loaded around player
    int loadDistance;        // Distance at which to start loading chunks
    int unloadDistance;      // Distance at which to unload chunks
    
    // Performance settings
    int maxLoadedChunks;     // Maximum number of chunks to keep in memory
    float chunkLoadInterval; // Time between chunk loading operations
    float lastLoadTime;
    
    // Player tracking
    Vec3 lastPlayerPosition;
    ChunkCoord lastPlayerChunk;
    
    // Callbacks
    std::function<void(const ChunkCoord&, const std::vector<TerrainBlockType>&)> onChunkGenerated;
    std::function<void(const ChunkCoord&)> onChunkUnloaded;

public:
    // Constructor
    InfiniteTerrainGenerator(const TerrainParams& params = TerrainParams());
    
    // Core functionality
    void update(const Vec3& playerPosition, float deltaTime);
    void generateChunk(const ChunkCoord& coord);
    void unloadChunk(const ChunkCoord& coord);
    
    // Chunk access
    TerrainBlockType getBlockAtWorldPosition(const Vec3& worldPos) const;
    bool isChunkGenerated(const ChunkCoord& coord) const;
    bool isChunkLoaded(const ChunkCoord& coord) const;
    
    // Utility functions
    bool isBlockSolid(TerrainBlockType blockType) const;
    
    // Utility functions
    ChunkCoord worldToChunkCoord(const Vec3& worldPos) const;
    Vec3 chunkCoordToWorld(const ChunkCoord& coord) const;
    Vec2 getChunkOffset(const ChunkCoord& coord) const;
    float getDistanceToChunk(const Vec3& playerPos, const ChunkCoord& coord) const;
    
    // Configuration
    void setRenderDistance(int distance) { 
        renderDistance = distance; 
        loadDistance = distance * 2;  // Set load distance to 2x render distance for better coverage
        unloadDistance = distance * 3;  // Set unload distance to 3x render distance
    }
    void setMaxLoadedChunks(int max) { maxLoadedChunks = max; }
    void setChunkLoadInterval(float interval) { chunkLoadInterval = interval; }
    
    // Terrain parameters
    void setTerrainParams(const TerrainParams& params);
    const TerrainParams& getTerrainParams() const;
    
    // Force regeneration
    void forceRegenerateAllChunks();
    
    // Memory management
    void forceMemoryCleanup();
    void setMemoryLimits(int maxChunks, int renderDist);
    
    // Callbacks
    void setOnChunkGenerated(std::function<void(const ChunkCoord&, const std::vector<TerrainBlockType>&)> callback);
    void setOnChunkUnloaded(std::function<void(const ChunkCoord&)> callback);
    
    // Statistics
    int getLoadedChunkCount() const { return static_cast<int>(chunks.size()); }
    int getQueuedLoadCount() const { return static_cast<int>(chunkLoadQueue.size()); }
    int getQueuedUnloadCount() const { return static_cast<int>(chunkUnloadQueue.size()); }
    
    // Debug
    void printStatistics() const;

private:
    // Internal methods
    void updateChunkLoading(float deltaTime);
    void updateChunkUnloading();
    void queueChunkForLoading(const ChunkCoord& coord);
    void queueChunkForUnloading(const ChunkCoord& coord);
    bool shouldLoadChunk(const ChunkCoord& coord, const Vec3& playerPos) const;
    bool shouldUnloadChunk(const ChunkCoord& coord, const Vec3& playerPos) const;
    void cleanupOldChunks();
    std::vector<ChunkCoord> getChunksInRange(const Vec3& center, float radius) const;
};

} // namespace Engine
