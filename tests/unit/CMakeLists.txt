find_package(Catch2 3 REQUIRED)

# Test for hello_counter
add_executable(test_hello_counter test_hello_counter.cpp)
target_link_libraries(test_hello_counter 
    PRIVATE 
    hello_counter 
    Catch2::Catch2WithMain
)
add_test(NAME test_hello_counter COMMAND test_hello_counter)

# Test for uart_tx
add_executable(test_uart_tx test_uart_tx.cpp)
target_link_libraries(test_uart_tx 
    PRIVATE 
    uart_tx 
    Catch2::Catch2WithMain
)
add_test(NAME test_uart_tx COMMAND test_uart_tx)

# Test for apb_timer
add_executable(test_apb_timer test_apb_timer.cpp)
target_link_libraries(test_apb_timer
    PRIVATE 
    apb_timer 
    Catch2::Catch2WithMain
)
add_test(NAME test_apb_timer COMMAND test_apb_timer)

# Test for arbitrator
add_executable(test_arbitrator test_arbitrator.cpp)
target_link_libraries(test_arbitrator
    PRIVATE
    arbitrator
    Catch2::Catch2WithMain
)
add_test(NAME test_arbitrator COMMAND test_arbitrator)

# Test for simple_DMA
add_executable(test_simple_dma test_simple_dma.cpp)
target_link_libraries(test_simple_dma
    PRIVATE
    simple_DMA
    Catch2::Catch2WithMain
)
add_test(NAME test_simple_dma COMMAND test_simple_dma)