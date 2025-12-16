add_library(linux_sc_minilib
        sm_defs.cpp
        timeout.cpp
        ptime.cpp
        display.cpp)


target_include_directories (linux_sc_minilib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})