
add_library(hello_counter 
    src/hello_counter.cpp
    include/hello_counter.h
)

target_include_directories(hello_counter PUBLIC include)

target_link_libraries(hello_counter PUBLIC SystemC::systemc)