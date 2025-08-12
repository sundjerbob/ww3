/**
 * main.cpp - Application Entry Point
 * 
 * OVERVIEW:
 * Simple entry point for the game engine application.
 * Creates and runs the main Game engine instance.
 * 
 * CLEAN ARCHITECTURE:
 * - Minimal main function
 * - All logic encapsulated in Game class
 * - Proper error handling and cleanup
 */

#include "Engine/Core/Game.h"
#include <iostream>

int main() {
    // Create game engine instance
    Engine::Game game(1200, 800, "Counter-Strike Style FPS Engine");
    
    // Initialize engine
    if (!game.initialize()) {
        std::cerr << "Failed to initialize game engine!" << std::endl;
        return -1;
    }
    
    // Run main game loop
    game.run();
    
    // Cleanup is handled automatically by Game destructor
    return 0;
}