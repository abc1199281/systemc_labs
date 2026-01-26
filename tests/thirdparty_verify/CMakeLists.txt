find_package(Catch2 3 REQUIRED)

# Test for tlm_memory
add_executable(test_tlm_memory 
    tlm/common/test_memory.cpp
    ../../thirdparty/accellera/tlm/common/src/memory.cpp
    ../../thirdparty/accellera/tlm/common/src/report.cpp
)

target_include_directories(test_tlm_memory 
    PRIVATE 
    ../../thirdparty/accellera/tlm/common/include
    ../../thirdparty/accellera/tlm/common/include/tlm
)

target_link_libraries(test_tlm_memory 
    PRIVATE 
    Catch2::Catch2
    SystemC::systemc
)

add_test(NAME test_tlm_memory COMMAND test_tlm_memory)

# Test for traffic_generator
add_executable(test_traffic_gen 
    tlm/common/test_traffic_gen.cpp
    ../../thirdparty/accellera/tlm/common/src/traffic_generator.cpp
    ../../thirdparty/accellera/tlm/common/src/report.cpp
)

target_include_directories(test_traffic_gen 
    PRIVATE 
    ../../thirdparty/accellera/tlm/common/include
    ../../thirdparty/accellera/tlm/common/include/tlm
)

target_link_libraries(test_traffic_gen 
    PRIVATE 
    Catch2::Catch2
    SystemC::systemc
)

add_test(NAME test_traffic_gen COMMAND test_traffic_gen)

# Test for SimpleBusLT
add_executable(test_tlm_simple_bus_lt
    tlm/common/models/test_simple_bus_lt.cpp
    ../../thirdparty/accellera/tlm/common/src/memory.cpp
    ../../thirdparty/accellera/tlm/common/src/report.cpp
)

target_include_directories(test_tlm_simple_bus_lt
    PRIVATE
    ../../thirdparty/accellera/tlm/common/include
    ../../thirdparty/accellera/tlm/common/include/tlm
)

target_link_libraries(test_tlm_simple_bus_lt
    PRIVATE
    Catch2::Catch2
    SystemC::systemc
)

add_test(NAME test_tlm_simple_bus_lt COMMAND test_tlm_simple_bus_lt)

# Test for LT Example Recreation
add_executable(test_tlm_lt
    tlm/lt/test_lt.cpp
    ../../thirdparty/accellera/tlm/lt/src/lt_top.cpp
    ../../thirdparty/accellera/tlm/lt/src/initiator_top.cpp
    ../../thirdparty/accellera/tlm/common/src/lt_initiator.cpp
    ../../thirdparty/accellera/tlm/common/src/lt_target.cpp
    ../../thirdparty/accellera/tlm/common/src/at_target_1_phase.cpp
    ../../thirdparty/accellera/tlm/common/src/traffic_generator.cpp
    ../../thirdparty/accellera/tlm/common/src/memory.cpp
    ../../thirdparty/accellera/tlm/common/src/report.cpp
)

target_include_directories(test_tlm_lt 
    PRIVATE 
    tlm/lt
    ../../thirdparty/accellera/tlm/lt/include
    ../../thirdparty/accellera/tlm/common/include
    ../../thirdparty/accellera/tlm/common/include/tlm
)

target_link_libraries(test_tlm_lt 
    PRIVATE 
    Catch2::Catch2
    SystemC::systemc
)

add_test(NAME test_tlm_lt COMMAND test_tlm_lt)
