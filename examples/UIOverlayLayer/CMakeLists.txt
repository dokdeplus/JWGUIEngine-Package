# A game of its own (Win32 + OpenGL) with JWGUIEngine's controls over its
# picture: UIOverlayLayer.
#
#   cmake -S . -B build -A x64 -DCMAKE_PREFIX_PATH=<package folder>
#   cmake --build build --config Release
#   build\Release\UIOverlayLayer.exe
#
# Inside the package, CMAKE_PREFIX_PATH can be left out: the folder two levels
# up is searched too.
cmake_minimum_required(VERSION 3.20)
project(UIOverlayLayer LANGUAGES CXX)

find_package(JWGUIEngine 1.0 REQUIRED
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable(UIOverlayLayer Main.cpp)
# The game draws with OpenGL itself.
target_link_libraries(UIOverlayLayer PRIVATE JWGUIEngine::JWGUIEngine opengl32)
# Windows program, the engine's runtime settings, JWGUIEngine.dll next to the
# exe, and the program's icon.
jwgui_configure_app(UIOverlayLayer ICON UIOverlayLayer.ico)
