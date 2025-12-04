find_package(Catch2 3 REQUIRED)

add_executable(run_unit_tests test_hello_counter.cpp)

target_link_libraries(run_unit_tests 
    PRIVATE 
    hello_counter 
    Catch2::Catch2WithMain
)