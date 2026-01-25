find_package(Catch2 3 REQUIRED)

# Test for tlm_memory
add_executable(test_tlm_memory 
    tlm/common/test_memory.cpp
    ../../thirdparty/accellera/tlm/common/src/memory.cpp
)

target_include_directories(test_tlm_memory 
    PRIVATE 
    ../../thirdparty/accellera/tlm/common/include
)

target_link_libraries(test_tlm_memory 
    PRIVATE 
    Catch2::Catch2
    SystemC::systemc
)

add_test(NAME test_tlm_memory COMMAND test_tlm_memory)
