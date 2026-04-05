cmake_minimum_required(VERSION 3.21)

execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_TAG)
    set(GIT_TAG "0.0.0")
endif()

project(Pong VERSION ${GIT_TAG} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v0.1.36
)
# GLAD v0.1.x has cmake_minimum_required(VERSION 2.6) which CMake 4.x
# no longer allows. This cache variable is the same fix CMake suggests
# on the command line: -DCMAKE_POLICY_VERSION_MINIMUM=3.5
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE INTERNAL "")
FetchContent_MakeAvailable(glad)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.10
)
FetchContent_MakeAvailable(SDL3)

# SDL3_ttf uses find_package(SDL3) internally; point it at the FetchContent build
FetchContent_GetProperties(SDL3)
set(SDL3_DIR "${sdl3_BINARY_DIR}" CACHE PATH "" FORCE)

FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-3.2.2
)
set(SDL3TTF_VENDORED ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3_ttf)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.5
)
set(HTTPLIB_USE_WINHTTP_IF_AVAILABLE ON CACHE BOOL "" FORCE)
set(HTTPLIB_REQUIRE_OPENSSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(httplib)

find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3 OpenGL::GL)

add_executable(Pong
    src/main.cpp
    src/core/Window.cpp
    src/core/Application.cpp
    src/core/Logger.cpp
    src/renderer/Shader.cpp
    src/renderer/Buffer.cpp
    src/renderer/TextRenderer.cpp
    src/renderer/Renderer2D.cpp
    src/game/Collider.cpp
    src/game/BallState.cpp
    src/game/PaddleState.cpp
    src/game/CpuController.cpp
    src/game/Physics.cpp
    src/scenes/SceneManager.cpp
    src/scenes/MainMenuScene.cpp
    src/scenes/GameScene.cpp
    src/scenes/PauseScene.cpp
    src/scenes/CreditsScene.cpp
    third_party/stb_vorbis.cpp
    src/core/AudioManager.cpp
    src/core/Updater.cpp
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Pong PRIVATE PONG_DEV)
    target_sources(Pong PRIVATE src/scenes/DevMenu.cpp)
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(Pong PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_compile_definitions(Pong PRIVATE
    PONG_VERSION="${GIT_TAG}"
    PONG_GITHUB_REPO="daddandrea/pong_game"
    PONG_ARCHIVE_LINUX="pong-linux-x86_64"
    PONG_ARCHIVE_WINDOWS="pong-windows-x86_64"
)

target_include_directories(Pong PRIVATE src third_party)

target_link_libraries(Pong PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    OpenGL::GL
    glad
    glm::glm
    imgui
    httplib::httplib
)

target_compile_options(Pong PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_custom_command(TARGET Pong POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:Pong>/assets"
    COMMENT "Copying assets to build directory"
)

add_custom_command(TARGET Pong POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/scripts/update.sh"
        "${CMAKE_SOURCE_DIR}/scripts/update.bat"
        "$<TARGET_FILE_DIR:Pong>/"
    COMMENT "Copying update scripts to build directory"
)

if(WIN32)
    add_custom_command(TARGET Pong POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Pong>
            $<TARGET_FILE_DIR:Pong>
        COMMAND_EXPAND_LISTS
    )
endif()
