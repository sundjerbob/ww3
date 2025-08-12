# WW3 - 3D Game Engine

A 3D game engine built with OpenGL and C++, featuring a modular architecture with rendering, input handling, and game object management.

## Features

- **3D Rendering**: OpenGL-based rendering with shader support
- **Game Object System**: Modular component-based architecture
- **Input Handling**: Keyboard and mouse input management
- **Camera System**: 3D camera with movement and rotation
- **Resource Management**: OBJ model loading and texture support
- **UI Elements**: Crosshair and minimap rendering
- **Scene Management**: Multiple scene support

## Project Structure

```
WW3/
├── Source/
│   ├── Engine/           # Core engine components
│   │   ├── Core/        # Game, GameObject, Scene classes
│   │   ├── Input/       # Input handling system
│   │   ├── Math/        # Camera and math utilities
│   │   ├── Rendering/   # Renderers, shaders, meshes
│   │   └── Utils/       # OBJ loader and utilities
│   ├── GameObjects/     # Game-specific objects
│   └── main.cpp         # Entry point
├── Resources/
│   ├── Objects/         # 3D models (.obj files)
│   └── Shaders/         # GLSL shader files
├── Extern/              # External libraries (GLEW, GLFW)
└── Include/             # Header files
```

## Dependencies

- **GLEW**: OpenGL Extension Wrangler Library
- **GLFW**: OpenGL Framework
- **OpenGL**: Graphics API

## Building the Project

### Prerequisites
- Visual Studio 2019 or later
- Windows 10/11

### Build Instructions
1. Open `WW3.sln` in Visual Studio
2. Select your desired build configuration (Debug/Release)
3. Build the solution (Ctrl+Shift+B)
4. Run the executable from `x64/Debug/WW3.exe`

## Controls

- **WASD**: Move camera
- **Mouse**: Look around
- **ESC**: Exit application

## Development

This project uses a modular architecture where:
- **Engine** classes provide core functionality
- **GameObjects** implement specific game logic
- **Rendering** system handles all graphics operations
- **Input** system manages user interactions

## License

This project is open source. Feel free to contribute or modify as needed.
