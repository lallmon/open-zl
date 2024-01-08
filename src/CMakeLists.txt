cmake_minimum_required(VERSION 3.5)

project(open-zl LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    core/system.h core/system.cpp
    core/window.h core/window.cpp
    core/audio.h core/audio.cpp
    core/controls.h core/controls.cpp
    core/fonts/font.h core/fonts/font.cpp
    core/fonts/magofont3.cpp

    game/game.h game/game.cpp
    game/worldloader.h game/worldloader.cpp
    game/worldcell.h game/worldcell.cpp
    game/world.h game/world.cpp
    game/events.h game/events.cpp
    game/playerdata.h game/playerdata.cpp

    )

if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

set(SOURCES_THIRDPARTY
    thirdparty/glad/glad.c
    )

set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/thirdparty
    ${CMAKE_SOURCE_DIR}/thirdparty/luajit
    ${CMAKE_SOURCE_DIR}/thirdparty/glad
    ${CMAKE_SOURCE_DIR}/thirdparty/GLFW
    ${CMAKE_SOURCE_DIR}/thirdparty/KHR
    ${CMAKE_SOURCE_DIR}/thirdparty/PortAudio
    )

add_executable(open-zl ${SOURCES_THIRDPARTY} ${SOURCES})

target_include_directories(open-zl PRIVATE ${INCLUDE_DIRS})

if (UNIX)
    add_compile_options(-Wall)
elseif(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT ")
endif()

if (UNIX)
    target_link_libraries(open-zl ${PROJECT_SOURCE_DIR}/libs/linux/libglfw3.a)
    target_link_libraries(open-zl ${PROJECT_SOURCE_DIR}/libs/linux/libportaudio.a)
    target_link_libraries(open-zl X11)
    target_link_libraries(open-zl pthread)
    target_link_libraries(open-zl Xrandr)
    target_link_libraries(open-zl Xi)
    target_link_libraries(open-zl dl)
    target_link_libraries(open-zl asound)
elseif(WIN32)
    target_link_libraries(open-zl ${PROJECT_SOURCE_DIR}/libs/win_x86_64/glfw3.lib)
    target_link_libraries(open-zl ${PROJECT_SOURCE_DIR}/libs/win_x86_64/portaudio.lib)
    target_link_libraries(open-zl ${PROJECT_SOURCE_DIR}/libs/win_x86_64/luajit.lib)
    target_link_libraries(open-zl ${PROJECT_SOURCE_DIR}/libs/win_x86_64/lua51.lib)
    target_link_libraries(open-zl user32 gdi32 shell32)
endif()
