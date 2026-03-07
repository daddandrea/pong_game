cmake_minimum_required(VERSION 3.20)
project(Pong CXX)

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

find_package(SDL3     REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(OpenGL   REQUIRED)
find_package(glm      REQUIRED CONFIG)

add_executable(Pong
    src/main.cpp
    src/core/Window.cpp
    src/core/Application.cpp
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
)

target_include_directories(Pong PRIVATE src third_party)

target_link_libraries(Pong PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    OpenGL::GL
    glad
    glm::glm
)

target_compile_options(Pong PRIVATE -Wall -Wextra -Wpedantic)

add_custom_command(TARGET Pong POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:Pong>/assets"
    COMMENT "Copying assets to build directory"
)
