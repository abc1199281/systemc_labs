add_library(arbitrator
    src/arbitrator.cpp
)

target_link_libraries(arbitrator
    PUBLIC
    SystemC::systemc
)

target_include_directories(arbitrator
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
