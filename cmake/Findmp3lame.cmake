# - Try to find libmp3lame
# Once done, this module will defines
# MP3LAME_LIBRARY, the name of the library to link against
# MP3LAME_FOUND, if false, do not try to link
# MP3LAME_INCLUDE_DIR, where to find header

set(MP3LAME_FOUND "NO")

if (WIN32)
    find_path(MP3LAME_INCLUDE_DIR lame/lame.h
            HINTS
            PATH_SUFFIXES include
            PATHS
            C:/lame
            C:/liblame
            E:/Work/lame-3.99.5/
            E:/Work/lame-3.99.5/output/Debug
            E:/Work/lame-3.99.5/output/Release
            )
else ()
    find_path(MP3LAME_INCLUDE_DIR lame/lame.h
            HINTS
            PATH_SUFFIXES include
            PATHS
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local/include
            /usr/include
            /sw/include
            /opt/local/include
            /opt/csw/include
            /opt/include
            /mingw
            /opt/lame/include
            /opt/lame
            )
endif ()


find_library(MP3LAME_LIBRARY
        NAMES libmp3lame.a
        HINTS
        PATH_SUFFIXES lib64 lib
        PATHS
        /usr/lib/x86_64-linux-gnu/
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        /mingw
        )

if (MP3LAME_LIBRARY)
    set(MP3LAME_FOUND "YES")
endif (MP3LAME_LIBRARY)
