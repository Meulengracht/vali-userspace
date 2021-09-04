cmake_minimum_required(VERSION 3.8)
project(SdlShooter C)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configure primary executable target
add_executable(shooter 
    src/bullets.c
    src/camera.c
    src/draw.c
    src/effects.c
    src/enemies.c
    src/entities.c
    src/highscores.c
    src/init.c
    src/input.c
    src/items.c
    src/main.c
    src/player.c
    src/sound.c
    src/stage.c
    src/text.c
    src/title.c
    src/util.c
)
target_link_libraries(shooter SDL2 SDL2_image SDL2_ttf SDL2_mixer)

# Add a new install target
install(TARGETS shooter
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY gfx DESTINATION share/shooter)
install(DIRECTORY music DESTINATION share/shooter)
install(DIRECTORY sound DESTINATION share/shooter)
