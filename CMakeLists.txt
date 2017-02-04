cmake_minimum_required(VERSION 3.6)
project(cinemo)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/main.cpp src/utils.cpp include/utils.hpp src/LameWrapper.cpp include/LameWrapper.hpp)
add_executable(cinemo ${SOURCE_FILES})

find_package(mp3lame REQUIRED)
if (MP3LAME_FOUND)
    set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${MP3LAME_LIBRARY})
    message(STATUS "MP3LAME Library was found in " ${MP3LAME_LIBRARY})
    target_link_libraries(cinemo mp3lame)
endif (MP3LAME_FOUND)
