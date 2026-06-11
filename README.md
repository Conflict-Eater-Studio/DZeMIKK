# DZeMIKK

## Tech Stack

- **Language:** C, C++20
- **Build System:** CMake 4.3.0
- **Toolchain (Windows):** MSVC (Visual Studio 2022) 17.0, Ninja 1.13.2
- **Graphics:** OpenGL (`opengl32`)
- **Libraries:** GLFW, GLAD, Assimp, Freetype, stb
- **Static Analysis / Style:** clang-tidy, clang-format
 ## Build Scripts (Windows)

The project provides batch scripts for common build configurations:

- `compile/compile.exe`

### Build Limitations

- **Release configuration excludes development tools.**
  The following components are available only in:
  - Debug
  - RelWithDebInfo

  Excluded from Release builds:
  - ImGui debug interface
  - FMOD debug library (`fmodL_vc.lib`)
  - spdlog development logging system