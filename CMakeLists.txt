cmake_minimum_required(VERSION 3.5)
project(cinemo)

set(CMAKE_CXX_STANDARD 11)

#platform specific stuff
if (WIN32)
    add_definitions(-DWIN32)
else () #asume linux
    add_definitions(-DLINUX)
    add_definitions(-DDEBUG)
endif ()

#headers and sources
include_directories(include)
set(SOURCE_FILES
        src/main.cpp
        src/utils.cpp
        src/LameWrapper.cpp
        src/wave_header.cpp
        )

add_executable(wav2mp3 ${SOURCE_FILES})

#Finding lame library
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}}/cmake)
include(./cmake/Findmp3lame.cmake)
if (WIN32)
    find_library(libmp3lame REQUIRED)
else ()
    find_package(mp3lame REQUIRED)
endif ()

if (MP3LAME_FOUND)
    set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${MP3LAME_LIBRARY})
    message(STATUS "MP3LAME Library was found in " ${MP3LAME_LIBRARY})
    target_link_libraries(wav2mp3 mp3lame)
elseif (!MP3LAM_FOUND)
    message(ERROR "mp3lame library is necessary!")
endif (MP3LAME_FOUND)
