# find_package(JWGUIEngine) for the JWGUIEngine release package.
#
#   find_package(JWGUIEngine 1.0 REQUIRED)            # CMAKE_PREFIX_PATH = package folder
#   add_executable(MyApp Main.cpp)
#   target_link_libraries(MyApp PRIVATE JWGUIEngine::JWGUIEngine)
#   jwgui_configure_app(MyApp)
#
# The engine's interface passes C++ standard library types, so an app must be
# built with MSVC (x64) and the engine's settings: static runtime (/MT) in
# every configuration and _ITERATOR_DEBUG_LEVEL=0. JWGUIEngine::JWGUIEngine
# carries the definitions; jwgui_configure_app sets the runtime.

if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(JWGUIEngine_FOUND FALSE)
    set(JWGUIEngine_NOT_FOUND_MESSAGE "JWGUIEngine needs MSVC for x64 (generator option -A x64).")
    return()
endif()

# lib/cmake/JWGUIEngine -> package root
get_filename_component(JWGUIEngine_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(JWGUIEngine_INCLUDE_DIR "${JWGUIEngine_ROOT_DIR}/include")

if(NOT TARGET JWGUIEngine::JWGUIEngine)
    add_library(JWGUIEngine::JWGUIEngine SHARED IMPORTED)
    set_target_properties(JWGUIEngine::JWGUIEngine PROPERTIES
        IMPORTED_LOCATION "${JWGUIEngine_ROOT_DIR}/bin/JWGUIEngine.dll"
        IMPORTED_IMPLIB "${JWGUIEngine_ROOT_DIR}/lib/JWGUIEngine.lib"
        # Both forms work: #include "UIButton.h" (as in the guides) and
        # #include <JWGUIEngine/Controls/UIButton.h>.
        INTERFACE_INCLUDE_DIRECTORIES
            "${JWGUIEngine_INCLUDE_DIR};${JWGUIEngine_INCLUDE_DIR}/JWGUIEngine/Core;${JWGUIEngine_INCLUDE_DIR}/JWGUIEngine/Controls;${JWGUIEngine_INCLUDE_DIR}/JWGUIEngine/Layout;${JWGUIEngine_INCLUDE_DIR}/JWGUIEngine/Scene"
        INTERFACE_COMPILE_DEFINITIONS
            "NOMINMAX;UNICODE;_UNICODE;$<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>"
        # C4251: standard library members of exported classes, expected with
        # this DLL (the engine's own build turns it off too).
        INTERFACE_COMPILE_OPTIONS "/utf-8;/EHsc;/wd4251"
        INTERFACE_COMPILE_FEATURES cxx_std_17
        # EntryPoint.h uses shell32 (CommandLineToArgvW).
        INTERFACE_LINK_LIBRARIES "user32;gdi32;shell32;ole32;comdlg32"
    )
endif()

# A Windows program (EntryPoint.h gives it wWinMain) with the engine's runtime
# settings, and JWGUIEngine.dll copied next to it after each build.
#   jwgui_configure_app(MyApp ICON app.ico PROJECT myapp.jwui)
# ICON embeds an .ico: Explorer shows it, and the window's title bar and
# taskbar button use it.
# PROJECT puts the project (.jwui and its res folder) inside the program,
# encrypted with the project's package key (bin\JWGUIPack.exe after each
# build): the program ships as its .exe alone and opens it when no project
# path is set. Editing the project rebuilds the program.
# KEY_FROM puts only the project's key inside the program, for an app that
# opens a protected .jwpack exported by the editor and shipped next to it.
function(jwgui_configure_app TARGET_NAME)
    cmake_parse_arguments(ARG "" "ICON;PROJECT;KEY_FROM" "" ${ARGN})
    get_target_property(engine_dll JWGUIEngine::JWGUIEngine IMPORTED_LOCATION)
    get_filename_component(engine_bin "${engine_dll}" DIRECTORY)
    set(pack_tool "${engine_bin}/JWGUIPack.exe")
    if(ARG_PROJECT)
        get_filename_component(project "${ARG_PROJECT}" ABSOLUTE)
        if(NOT EXISTS "${project}")
            message(FATAL_ERROR "jwgui_configure_app: project not found: ${project}")
        endif()
        get_filename_component(project_dir "${project}" DIRECTORY)
        file(GLOB_RECURSE project_files CONFIGURE_DEPENDS "${project_dir}/res/*")
        # A change to the project relinks the program, which embeds it again.
        set_property(TARGET ${TARGET_NAME} APPEND PROPERTY LINK_DEPENDS "${project};${project_files}")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND "${pack_tool}" project "${project}" "$<TARGET_FILE:${TARGET_NAME}>"
            COMMENT "Put the project inside ${TARGET_NAME}, encrypted"
        )
    elseif(ARG_KEY_FROM)
        get_filename_component(project "${ARG_KEY_FROM}" ABSOLUTE)
        set_property(TARGET ${TARGET_NAME} APPEND PROPERTY LINK_DEPENDS "${project}")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND "${pack_tool}" key "${project}" "$<TARGET_FILE:${TARGET_NAME}>"
            COMMENT "Put the package key inside ${TARGET_NAME}"
        )
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
        MSVC_RUNTIME_LIBRARY "MultiThreaded"
    )
    if(ARG_ICON)
        get_filename_component(icon "${ARG_ICON}" ABSOLUTE)
        if(NOT EXISTS "${icon}")
            message(FATAL_ERROR "jwgui_configure_app: icon not found: ${icon}")
        endif()
        set(rc "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_icon.rc")
        file(GENERATE OUTPUT "${rc}" CONTENT "1 ICON \"${icon}\"\n")
        target_sources(${TARGET_NAME} PRIVATE "${rc}")
        set_property(SOURCE "${rc}" APPEND PROPERTY OBJECT_DEPENDS "${icon}")
    endif()
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:JWGUIEngine::JWGUIEngine>" "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        COMMENT "Copy JWGUIEngine.dll next to ${TARGET_NAME}"
    )
endfunction()

set(JWGUIEngine_FOUND TRUE)
