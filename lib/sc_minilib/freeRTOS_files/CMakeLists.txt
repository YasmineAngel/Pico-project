add_library(rtos_sc_minilib
        sm_defs.cpp
        timeout.cpp
        ptime.cpp)

add_compile_options(-fexceptions)

target_include_directories (rtos_sc_minilib PUBLIC includes)

target_link_libraries(rtos_sc_minilib
        pico_stdlib
        pico_cyw43_arch_none
        FreeRTOS-Kernel-Heap4
)
