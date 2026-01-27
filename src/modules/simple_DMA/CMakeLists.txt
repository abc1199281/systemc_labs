
add_library(simple_DMA
    src/simple_dma.cpp
    include/simple_dma.h
    include/ideal_memory.h
)

target_include_directories(simple_DMA PUBLIC include)

target_link_libraries(simple_DMA PUBLIC SystemC::systemc)
