/**
 * InfiniteTerrainGenerator.cpp - Infinite Terrain Generation Implementation
 * 
 * Implements infinite terrain generation using Perlin noise from CProceduralGame.
 * Manages dynamic chunk loading/unloading for seamless infinite worlds.
 */

#include "InfiniteTerrainGenerator.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Engine {

InfiniteTerrainGenerator::InfiniteTerrainGenerator(const TerrainParams& params)
    : terrainGenerator(params), chunkSize(16), chunkHeight(32), // Further reduced height to save memory
      renderDistance(3), loadDistance(5), unloadDistance(8), // More conservative distances for stability
      maxLoadedChunks(32), chunkLoadInterval(0.05f), lastLoadTime(0.0f), // Reduced max chunks, slower loading
      lastPlayerPosition(0.0f, 0.0f, 0.0f), lastPlayerChunk(0, 0) {
}

void InfiniteTerrainGenerator::update(const Vec3& playerPosition, float deltaTime) {
    // Update player tracking
    lastPlayerPosition = playerPosition;
    ChunkCoord currentPlayerChunk = worldToChunkCoord(playerPosition);
    
    // Check if player moved to a new chunk
    if (currentPlayerChunk.x != lastPlayerChunk.x || currentPlayerChunk.z != lastPlayerChunk.z) {
        lastPlayerChunk = currentPlayerChunk;
    }
    
    // Update chunk loading/unloading
    updateChunkLoading(deltaTime);
    updateChunkUnloading();
    
    // Cleanup old chunks if we exceed memory limit
    if (static_cast<int>(chunks.size()) > maxLoadedChunks) {
        cleanupOldChunks();
    }
}

void InfiniteTerrainGenerator::updateChunkLoading(float deltaTime) {
    lastLoadTime += deltaTime;
    
    // Process chunk loading at intervals to avoid frame drops
    if (lastLoadTime < chunkLoadInterval) {
        return;
    }
    
    lastLoadTime = 0.0f;
    
    // Get chunks that should be loaded around player
    std::vector<ChunkCoord> chunksToLoad = getChunksInRange(lastPlayerPosition, static_cast<float>(loadDistance));
    
    // Queue chunks for loading (reduced debug output)
    int queuedCount = 0;
    for (const auto& coord : chunksToLoad) {
        if (!isChunkGenerated(coord) && !isChunkLoaded(coord)) {
            queueChunkForLoading(coord);
            queuedCount++;
        }
    }
    
    // Chunks queued for loading
    
    // Process one chunk from the load queue
    if (!chunkLoadQueue.empty()) {
        ChunkCoord coord = chunkLoadQueue.front();
        chunkLoadQueue.pop();
        generateChunk(coord);
    }
}

void InfiniteTerrainGenerator::updateChunkUnloading() {
    // Get chunks that should be unloaded
    std::vector<ChunkCoord> chunksToUnload;
    
    for (const auto& pair : chunks) {
        const ChunkCoord& coord = pair.first;
        if (shouldUnloadChunk(coord, lastPlayerPosition)) {
            chunksToUnload.push_back(coord);
        }
    }
    
    // Queue chunks for unloading
    for (const auto& coord : chunksToUnload) {
        queueChunkForUnloading(coord);
    }
    
    // Process chunks from the unload queue (less aggressive for stability)
    int chunksProcessed = 0;
    while (!chunkUnloadQueue.empty() && chunksProcessed < 1) { // Process only 1 chunk per frame
        ChunkCoord coord = chunkUnloadQueue.front();
        chunkUnloadQueue.pop();
        unloadChunk(coord);
        chunksProcessed++;
    }
    
    // Force cleanup if we're still over the limit
    if (static_cast<int>(chunks.size()) > maxLoadedChunks) {
        cleanupOldChunks();
    }
}

void InfiniteTerrainGenerator::generateChunk(const ChunkCoord& coord) {
    // Check if chunk already exists
    auto it = chunks.find(coord);
    if (it != chunks.end() && it->second.isGenerated) {
        return;
    }
    
    // Create new chunk data
    TerrainChunkData& chunkData = chunks[coord];
    chunkData.isGenerated = true;
    chunkData.isLoaded = true;
    chunkData.lastAccessTime = 0.0f;
    
    // Generate terrain for this chunk
    Vec2 chunkOffset = getChunkOffset(coord);
    terrainGenerator.generateChunkTerrain(chunkData.blocks, chunkSize, chunkHeight, chunkOffset);
    
    // Notify callback
    if (onChunkGenerated) {
        onChunkGenerated(coord, chunkData.blocks);
    }
    
    // Chunk generated successfully
}

void InfiniteTerrainGenerator::unloadChunk(const ChunkCoord& coord) {
    auto it = chunks.find(coord);
    if (it != chunks.end()) {
        // Notify callback
        if (onChunkUnloaded) {
            onChunkUnloaded(coord);
        }
        
        // Remove chunk data
        chunks.erase(it);
        
        // Chunk unloaded
    }
}

TerrainBlockType InfiniteTerrainGenerator::getBlockAtWorldPosition(const Vec3& worldPos) const {
    ChunkCoord coord = worldToChunkCoord(worldPos);
    
    auto it = chunks.find(coord);
    if (it == chunks.end() || !it->second.isGenerated) {
        return TerrainBlockType::Air; // Return air for ungenerated chunks
    }
    
    // Calculate local position within chunk
    int localX = static_cast<int>(std::floor(worldPos.x)) % chunkSize;
    int localZ = static_cast<int>(std::floor(worldPos.z)) % chunkSize;
    int localY = static_cast<int>(std::floor(worldPos.y));
    
    // Handle negative coordinates
    if (localX < 0) localX += chunkSize;
    if (localZ < 0) localZ += chunkSize;
    
    // Bounds checking
    if (localX < 0 || localX >= chunkSize || localZ < 0 || localZ >= chunkSize || 
        localY < 0 || localY >= chunkHeight) {
        return TerrainBlockType::Air;
    }
    
    // Get block from chunk data
    int blockIndex = localY * chunkSize * chunkSize + localZ * chunkSize + localX;
    if (blockIndex >= 0 && blockIndex < static_cast<int>(it->second.blocks.size())) {
        return it->second.blocks[blockIndex];
    }
    
    return TerrainBlockType::Air;
}

bool InfiniteTerrainGenerator::isChunkGenerated(const ChunkCoord& coord) const {
    auto it = chunks.find(coord);
    return it != chunks.end() && it->second.isGenerated;
}

bool InfiniteTerrainGenerator::isChunkLoaded(const ChunkCoord& coord) const {
    auto it = chunks.find(coord);
    return it != chunks.end() && it->second.isLoaded;
}

bool InfiniteTerrainGenerator::isBlockSolid(TerrainBlockType blockType) const {
    return terrainGenerator.isBlockSolid(blockType);
}

ChunkCoord InfiniteTerrainGenerator::worldToChunkCoord(const Vec3& worldPos) const {
    int chunkX = static_cast<int>(std::floor(worldPos.x / static_cast<float>(chunkSize)));
    int chunkZ = static_cast<int>(std::floor(worldPos.z / static_cast<float>(chunkSize)));
    return ChunkCoord(chunkX, chunkZ);
}

Vec3 InfiniteTerrainGenerator::chunkCoordToWorld(const ChunkCoord& coord) const {
    float worldX = static_cast<float>(coord.x * chunkSize);
    float worldZ = static_cast<float>(coord.z * chunkSize);
    return Vec3(worldX, 0.0f, worldZ);
}

Vec2 InfiniteTerrainGenerator::getChunkOffset(const ChunkCoord& coord) const {
    float offsetX = static_cast<float>(coord.x * chunkSize);
    float offsetZ = static_cast<float>(coord.z * chunkSize);
    return Vec2(offsetX, offsetZ);
}

float InfiniteTerrainGenerator::getDistanceToChunk(const Vec3& playerPos, const ChunkCoord& coord) const {
    Vec3 chunkCenter = chunkCoordToWorld(coord);
    chunkCenter.x += static_cast<float>(chunkSize) / 2.0f;
    chunkCenter.z += static_cast<float>(chunkSize) / 2.0f;
    
    Vec3 diff = playerPos - chunkCenter;
    float distance = std::sqrt(diff.x * diff.x + diff.z * diff.z);
    
    return distance;
}

void InfiniteTerrainGenerator::setTerrainParams(const TerrainParams& params) {
    // Update the terrain generator parameters
    terrainGenerator.setParams(params);
    
    // Force regeneration of all chunks
    forceRegenerateAllChunks();
}

void InfiniteTerrainGenerator::forceRegenerateAllChunks() {
    // Clear all existing chunks to force regeneration with new parameters
    std::cout << "InfiniteTerrainGenerator: Force regenerating all chunks..." << std::endl;
    chunks.clear();
    chunkLoadQueue = std::queue<ChunkCoord>();  // Clear load queue
    chunkUnloadQueue = std::queue<ChunkCoord>(); // Clear unload queue
    
    // Reset player tracking to force new chunk generation
    lastPlayerPosition = Vec3(0.0f, 0.0f, 0.0f);
    lastPlayerChunk = ChunkCoord(0, 0);
    
    std::cout << "InfiniteTerrainGenerator: All chunks cleared, ready for regeneration" << std::endl;
}

void InfiniteTerrainGenerator::forceMemoryCleanup() {
    std::cout << "InfiniteTerrainGenerator: Force memory cleanup..." << std::endl;
    
    // Force cleanup of old chunks
    cleanupOldChunks();
    
    // Clear queues
    while (!chunkLoadQueue.empty()) {
        chunkLoadQueue.pop();
    }
    while (!chunkUnloadQueue.empty()) {
        chunkUnloadQueue.pop();
    }
    
    std::cout << "InfiniteTerrainGenerator: Memory cleanup completed. Chunks: " << chunks.size() << std::endl;
}

void InfiniteTerrainGenerator::setMemoryLimits(int maxChunks, int renderDist) {
    maxLoadedChunks = maxChunks;
    renderDistance = renderDist;
    loadDistance = renderDist * 2;
    unloadDistance = renderDist * 3;
    
    std::cout << "InfiniteTerrainGenerator: Memory limits updated - Max chunks: " << maxChunks 
              << ", Render distance: " << renderDist << std::endl;
}

const TerrainParams& InfiniteTerrainGenerator::getTerrainParams() const {
    return terrainGenerator.getParams();
}

void InfiniteTerrainGenerator::setOnChunkGenerated(std::function<void(const ChunkCoord&, const std::vector<TerrainBlockType>&)> callback) {
    onChunkGenerated = callback;
}

void InfiniteTerrainGenerator::setOnChunkUnloaded(std::function<void(const ChunkCoord&)> callback) {
    onChunkUnloaded = callback;
}

void InfiniteTerrainGenerator::queueChunkForLoading(const ChunkCoord& coord) {
    // Check if already queued
    std::queue<ChunkCoord> tempQueue = chunkLoadQueue;
    while (!tempQueue.empty()) {
        if (tempQueue.front() == coord) {
            return; // Already queued
        }
        tempQueue.pop();
    }
    
    chunkLoadQueue.push(coord);
}

void InfiniteTerrainGenerator::queueChunkForUnloading(const ChunkCoord& coord) {
    // Check if already queued
    std::queue<ChunkCoord> tempQueue = chunkUnloadQueue;
    while (!tempQueue.empty()) {
        if (tempQueue.front() == coord) {
            return; // Already queued
        }
        tempQueue.pop();
    }
    
    chunkUnloadQueue.push(coord);
}

bool InfiniteTerrainGenerator::shouldLoadChunk(const ChunkCoord& coord, const Vec3& playerPos) const {
    float distance = getDistanceToChunk(playerPos, coord);
    return distance <= static_cast<float>(loadDistance);
}

bool InfiniteTerrainGenerator::shouldUnloadChunk(const ChunkCoord& coord, const Vec3& playerPos) const {
    float distance = getDistanceToChunk(playerPos, coord);
    return distance > static_cast<float>(unloadDistance);
}

void InfiniteTerrainGenerator::cleanupOldChunks() {
    // Find chunks with oldest access time
    std::vector<std::pair<float, ChunkCoord>> chunkAges;
    
    for (const auto& pair : chunks) {
        chunkAges.push_back({pair.second.lastAccessTime, pair.first});
    }
    
    // Sort by age (oldest first)
    std::sort(chunkAges.begin(), chunkAges.end());
    
    // Remove oldest chunks until we're under the limit
    int chunksToRemove = static_cast<int>(chunks.size()) - maxLoadedChunks;
    int removed = 0;
    for (int i = 0; i < chunksToRemove && i < static_cast<int>(chunkAges.size()); i++) {
        unloadChunk(chunkAges[i].second);
        removed++;
    }
    
    if (removed > 0) {
        std::cout << "Memory cleanup: Removed " << removed << " old chunks. Current chunks: " << chunks.size() << std::endl;
    }
}

std::vector<ChunkCoord> InfiniteTerrainGenerator::getChunksInRange(const Vec3& center, float radius) const {
    std::vector<ChunkCoord> chunksInRange;
    
    // Calculate chunk range
    int minChunkX = static_cast<int>(std::floor((center.x - radius) / static_cast<float>(chunkSize)));
    int maxChunkX = static_cast<int>(std::floor((center.x + radius) / static_cast<float>(chunkSize)));
    int minChunkZ = static_cast<int>(std::floor((center.z - radius) / static_cast<float>(chunkSize)));
    int maxChunkZ = static_cast<int>(std::floor((center.z + radius) / static_cast<float>(chunkSize)));
    
    // Add all chunks in range (reduced debug output)
    for (int x = minChunkX; x <= maxChunkX; x++) {
        for (int z = minChunkZ; z <= maxChunkZ; z++) {
            ChunkCoord coord(x, z);
            float distance = getDistanceToChunk(center, coord);
            if (distance <= radius) {
                chunksInRange.push_back(coord);
            }
        }
    }
    
    return chunksInRange;
}

void InfiniteTerrainGenerator::printStatistics() const {
    std::cout << "=== Infinite Terrain Statistics ===" << std::endl;
    std::cout << "Loaded chunks: " << getLoadedChunkCount() << std::endl;
    std::cout << "Queued for loading: " << getQueuedLoadCount() << std::endl;
    std::cout << "Queued for unloading: " << getQueuedUnloadCount() << std::endl;
    std::cout << "Player chunk: (" << lastPlayerChunk.x << ", " << lastPlayerChunk.z << ")" << std::endl;
    std::cout << "Player position: (" << lastPlayerPosition.x << ", " << lastPlayerPosition.y << ", " << lastPlayerPosition.z << ")" << std::endl;
    std::cout << "===================================" << std::endl;
}

} // namespace Engine
