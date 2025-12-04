
add_library(uart_tx
    src/uart_tx.cpp
    include/uart_tx.h
)

target_include_directories(uart_tx PUBLIC include)

target_link_libraries(uart_tx PUBLIC SystemC::systemc)