cmake_minimum_required(VERSION 4.0)
set(NAME "2D-FluidSim")
project(${NAME})

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.1
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(${NAME}
        src/main.cpp
        src/simulator/fluidSim.cpp
        src/simulator/fluidSim.h
        src/rendering/fps_counter.cpp
        src/rendering/fps_counter.h
        src/rendering/grid_renderer.cpp
        src/rendering/grid_renderer.h
        src/simulator/particle.cpp
        src/simulator/particle.h
        src/main.cpp
)
target_compile_features(${NAME} PRIVATE cxx_std_17)
target_link_libraries(${NAME} PRIVATE SFML::Graphics)