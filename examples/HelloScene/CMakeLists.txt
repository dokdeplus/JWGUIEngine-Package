# A C++ app on the JWGUIEngine release package.
#
# Windows:
#   cmake -S . -B build -A x64 -DCMAKE_PREFIX_PATH=<package folder>
#   cmake --build build --config Release
#   build\Release\Hello.exe
# Linux and macOS:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<package folder>
#   cmake --build build
#   build/Hello                      (macOS: open build/Hello.app)
#
# Inside the package, CMAKE_PREFIX_PATH can be left out: the folder two levels
# up is searched too.
cmake_minimum_required(VERSION 3.20)
project(Hello LANGUAGES CXX)

find_package(JWGUIEngine 1.0 REQUIRED
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable(Hello Main.cpp)
target_link_libraries(Hello PRIVATE JWGUIEngine::JWGUIEngine)
# The engine's settings, the engine next to the program (Windows, Linux) or
# in its .app (macOS), and the program's icon.
jwgui_configure_app(Hello ICON HelloScene.ico)

# The project is read at run time; copy it next to the program.
add_custom_command(TARGET Hello POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/hello.jwui" "$<TARGET_FILE_DIR:Hello>/hello.jwui")
