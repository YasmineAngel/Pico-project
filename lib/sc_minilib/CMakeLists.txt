cmake_minimum_required(VERSION 3.13)

if(FREERTOS)
    add_subdirectory(freeRTOS_files)
    add_library(sc_minilib
            freeRTOS_files/sm_defs.cpp
            freeRTOS_files/timeout.cpp
            freeRTOS_files/ptime.cpp
            state_chart.cpp
            transition.cpp)

    add_compile_options(-fexceptions)
    target_include_directories (sc_minilib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(sc_minilib
            pico_stdlib
            rtos_sc_minilib)
else()
    add_subdirectory(linux_files)
    add_library(sc_minilib
            linux_files/sm_defs.cpp
            linux_files/timeout.cpp
            linux_files/ptime.cpp
            state_chart.cpp
            transition.cpp
            linux_files/display.cpp)


    target_include_directories (sc_minilib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(sc_minilib
            pthread
            linux_sc_minilib)
endif()
