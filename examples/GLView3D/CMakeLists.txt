# A desk globe in a GLView (the app's own OpenGL 3.3, glad from the engine's
# context) with UIOverlayLayer's settings over the 3D picture, a menu bar and
# a status bar. The map is earth.jpg (Natural Earth II, public domain).
#
# Windows:
#   cmake -S . -B build -A x64 -DCMAKE_PREFIX_PATH=<package folder>
#   cmake --build build --config Release
#   build\Release\GLView3D.exe
# Linux and macOS:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<package folder>
#   cmake --build build
#   build/GLView3D                   (macOS: open build/GLView3D.app)
#
# Inside the package, CMAKE_PREFIX_PATH can be left out: the folder two levels
# up is searched too.
cmake_minimum_required(VERSION 3.20)
project(GLView3D LANGUAGES CXX)

find_package(JWGUIEngine 1.0 REQUIRED
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../..")

set(MAP "${CMAKE_CURRENT_SOURCE_DIR}/earth.jpg")
add_executable(GLView3D Main.cpp)
# GL comes from the engine's context through glad (glad_gl.h is in include):
# nothing more to link.
target_link_libraries(GLView3D PRIVATE JWGUIEngine::JWGUIEngine)
jwgui_configure_app(GLView3D ICON GLView3D.ico)

# The map is read at run time: next to the program, or in the .app's
# Resources on macOS.
if(APPLE)
    target_sources(GLView3D PRIVATE "${MAP}")
    set_source_files_properties("${MAP}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
else()
    add_custom_command(TARGET GLView3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MAP}" "$<TARGET_FILE_DIR:GLView3D>/earth.jpg")
endif()
