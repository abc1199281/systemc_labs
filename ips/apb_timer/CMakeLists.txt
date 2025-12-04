
add_library(apb_timer
    src/apb_timer.cpp
    include/apb_timer.h
)

target_include_directories(apb_timer PUBLIC include)

target_link_libraries(apb_timer PUBLIC SystemC::systemc)