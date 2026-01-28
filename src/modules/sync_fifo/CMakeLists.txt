
add_library(sync_fifo INTERFACE)

target_include_directories(sync_fifo INTERFACE include)
# No source file needed for INTERFACE library if it's header-only templates

# Link SystemC
target_link_libraries(sync_fifo INTERFACE SystemC::systemc)
