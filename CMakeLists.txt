cmake_minimum_required(VERSION 3.5)
project(cinemo)

set(CMAKE_CXX_STANDARD 11)

#platform specific stuff
if (WIN32)
    add_definitions(-DWIN32)
    include_directories(include/win/)
else () #asume linux
    add_definitions(-DLINUX)
    #test with high optimization
    #    if (CMAKE_BUILD_TYPE EQUAL "Release")
    #        message(STATUS "Build type is Release under Linux")
    #        set(CMAKE_CXX_FLAGS "-O4") #optimize
    #        set(CMAKE_EXE_LINKER_FLAGS "-s") #strip symbols
    #    endif ()
    #    if (COPY_TEST_FILES EQUAL "no")
    #        message(STATUS "NO TEST!")
    #    endif ()
endif ()

#headers and sources
include_directories(include/)
set(SOURCE_FILES
        src/main.cpp
        src/args.cpp
        src/utils.cpp
        src/LameWrapper.cpp
        src/wave_header.cpp
        src/threading.cpp
        )

add_executable(wav2mp3 ${SOURCE_FILES})

#Finding lame library
set(LIBS_DIR_x86_RELESAE ./lib/Release/x86)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}}/cmake)
include(./cmake/Findmp3lame.cmake)
if (WIN32)
    find_library(MP3LAME_LIBRARY REQUIRED)
else ()
    #find_package(mp3lame REQUIRED)
    find_library(MP3LAME_LIBRARY mp3lame.a) #static lib
endif ()

if (MP3LAME_FOUND)
    set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${MP3LAME_LIBRARY})
    message(STATUS "MP3LAME Library was found in " ${MP3LAME_LIBRARY})
    if (WIN32)
        target_link_libraries(wav2mp3
                ${LIBS_DIR_x86_RELESAE}/libmp3lame-static
                ${LIBS_DIR_x86_RELESAE}/libmpghip-static
                ${LIBS_DIR_x86_RELESAE}/pthreadVC2)
    else ()
        target_link_libraries(wav2mp3 pthread mp3lame)
    endif ()
elseif (!MP3LAM_FOUND)
    message(ERROR "mp3lame library is necessary!")
endif (MP3LAME_FOUND)
