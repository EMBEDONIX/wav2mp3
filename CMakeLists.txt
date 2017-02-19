#CMake directives for project CINEMO/wav2mp3
#Copyright Feb. 2017, WWW.EMBEDONIX.COM
#Author: Saeid Yazdani

cmake_minimum_required(VERSION 3.5)
project(cinemo)

set(CMAKE_CXX_STANDARD 11)

#platform specific stuff
if (WIN32)

    #NOTES FOR COMPILING ON WINDOWS:
    #since lamelibrary and pthread for Windows included with this repository are
    #not compiled in debug mode, the only supported build configuration will be Release
    #feel free to add your own debug compiled libraries and change this cmake file.
    #Then, do not forget about library linking down below :P

    set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
    add_definitions(-DWIN32) #just to be sure ;)
    include_directories(include/win/) #Windows specific header files
    set(COPY_COMMAND "copy")    

else () #asume linux

    add_definitions(-DLINUX)   

endif ()

#general headers

include_directories(include/)

#source files
set(SOURCE_FILES
        src/main.cpp
        src/args.cpp
        src/utils.cpp
        src/LameWrapper.cpp
        src/wave_header.cpp
        src/threading.cpp
        )

add_executable(wav2mp3 ${SOURCE_FILES})

#Preparing libraries for linkage
if (WIN32)

    #for static linking
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

    #LIBS
    # change to ' ${CMAKE_CURRENT_SOURCE_DIR}/lib/ReleaseSSE2/x86 ' for SSE2 support
    set(LIBS_DIR_x86_RELESAE  ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release/x86)

    set(MP3LAME_LIBRARY ${LIBS_DIR_x86_RELESAE}/libmp3lame-static)
    set(${MP3LAME_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/win)

    set_target_properties (wav2mp3 PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt")

    #link Release
    target_link_libraries(wav2mp3
            #MSVCRT.LIB
            ${LIBS_DIR_x86_RELESAE}/pthreadVC2.lib
            ${LIBS_DIR_x86_RELESAE}/libmp3lame-static.lib
            ${LIBS_DIR_x86_RELESAE}/libmpghip-static.lib)

    #Since libpthread is not being linked statically, the dll file needs to be copied
    #in the output directory after build process
    add_custom_command(TARGET 
            wav2mp3
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${LIBS_DIR_x86_RELESAE}/pthreadVC2.dll ${PROJECT_BINARY_DIR}/Release)

else ()

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findmp3lame.cmake)
    find_library(MP3LAME_LIBRARY mp3lame.a) #static lib

    #MP3 LAME ON LINUX
    if (MP3LAME_FOUND)

        set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${MP3LAME_LIBRARY})
        message(STATUS "MP3LAME Library was found in " ${MP3LAME_LIBRARY})
        target_link_libraries(wav2mp3 pthread mp3lame)

    elseif (!MP3LAM_FOUND)

        message(ERROR "mp3lame library is necessary!")

    endif (MP3LAME_FOUND)

endif ()