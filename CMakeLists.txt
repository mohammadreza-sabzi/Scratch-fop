cmake_minimum_required(VERSION 4.0)
project(Scratch_fop)

set(CMAKE_CXX_STANDARD 20)

add_executable(Scratch_fop main.cpp
        utils.h
        render.h
        input.h
        globals.h
        structs.h
        engine.h
        audio.h)
target_link_libraries(${PROJECT_NAME} -lconio)
target_link_libraries(${PROJECT_NAME} -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer)
