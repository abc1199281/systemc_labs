#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <systemc>

// The reporting globals need to be defined in one compilation unit
#define REPORT_DEFINE_GLOBALS
#include "reporting.h"
#include "memory.h"
#include <tlm>

using namespace sc_core;

// Define sc_main which is required by SystemC
int sc_main(int argc, char* argv[]) {
    // Initialize reporting enables
    REPORT_ENABLE_ALL_REPORTING();
    
    int result = Catch::Session().run(argc, argv);
    return result;
}

// Helper to create a dummy payload
void setup_payload(tlm::tlm_generic_payload& gp, tlm::tlm_command cmd, uint64_t addr, unsigned char* data, unsigned int len) {
    gp.set_command(cmd);
    gp.set_address(addr);
    gp.set_data_ptr(data);
    gp.set_data_length(len);
    gp.set_streaming_width(len);
    gp.set_byte_enable_ptr(0);
    gp.set_dmi_allowed(false);
    gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
}

TEST_CASE("Memory Initialization", "[memory]") {
    sc_time read_delay(10, SC_NS);
    sc_time write_delay(20, SC_NS);
    uint64_t size = 1024;
    uint32_t width = 4;

    REQUIRE_NOTHROW(memory(1, read_delay, write_delay, size, width));
}

TEST_CASE("Memory Operation - Read/Write", "[memory]") {
    sc_time read_delay(10, SC_NS);
    sc_time write_delay(20, SC_NS);
    uint64_t size = 1024;
    uint32_t width = 4;
    memory mem(1, read_delay, write_delay, size, width);

    unsigned char data_buffer[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    tlm::tlm_generic_payload gp;
    sc_time delay = SC_ZERO_TIME;

    SECTION("Write Operation") {
        uint64_t addr = 0x20;
        setup_payload(gp, tlm::TLM_WRITE_COMMAND, addr, data_buffer, 4);
        mem.operation(gp, delay);
        REQUIRE(gp.get_response_status() == tlm::TLM_OK_RESPONSE);

        std::cout << "Write Data at " << std::hex << addr << ": ";
        unsigned char* mem_ptr = mem.get_mem_ptr();
        for (unsigned int i = 0; i < 4; ++i) {
            std::cout << std::hex << static_cast<int>(mem_ptr[addr + i]) << " ";
            REQUIRE(mem_ptr[addr + i] == data_buffer[i]);
        }
        std::cout << std::dec << std::endl;
        std::cout << "Write Delay: " << delay.to_string() << std::endl;
        REQUIRE(delay == write_delay);
    }

    SECTION("Read Operation") {
        uint64_t addr = 0x10;
        unsigned char* mem_ptr = mem.get_mem_ptr();
        // Backdoor write to memory to ensure we have something to read
        for(int i=0; i<4; i++) {
            mem_ptr[addr + i] = data_buffer[i];
        }

        // Clear data buffer to verify it gets populated
        unsigned char read_buffer[4] = {0};
        setup_payload(gp, tlm::TLM_READ_COMMAND, addr, read_buffer, 4);
        
        mem.operation(gp, delay);
        REQUIRE(gp.get_response_status() == tlm::TLM_OK_RESPONSE);
        
        std::cout << "Read Data from " << std::hex << addr << ": ";
        for (unsigned int i = 0; i < 4; ++i) {
            std::cout << std::hex << static_cast<int>(read_buffer[i]) << " ";
            REQUIRE(read_buffer[i] == data_buffer[i]);
        }
        std::cout << std::dec << std::endl;
        std::cout << "Read Delay: " << delay.to_string() << std::endl;
        REQUIRE(delay == read_delay);
    }
}

TEST_CASE("Memory Address Checking", "[memory]") {
    sc_time read_delay(10, SC_NS);
    sc_time write_delay(20, SC_NS);
    uint64_t size = 100; // Small size for testing bounds
    uint32_t width = 4;
    memory mem(2, read_delay, write_delay, size, width);

    unsigned char data_buffer[4] = {0};
    tlm::tlm_generic_payload gp;
    sc_time delay = SC_ZERO_TIME;

    SECTION("Valid Address") {
        setup_payload(gp, tlm::TLM_READ_COMMAND, 0x0, data_buffer, 4);
        mem.operation(gp, delay);
        REQUIRE(gp.get_response_status() == tlm::TLM_OK_RESPONSE);
    }

    SECTION("Invalid Address - Out of Range") {
        setup_payload(gp, tlm::TLM_READ_COMMAND, size, data_buffer, 1);
        mem.operation(gp, delay);
        REQUIRE(gp.get_response_status() == tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }

    SECTION("Invalid Size - Burst Beyond Range") {
        setup_payload(gp, tlm::TLM_READ_COMMAND, size + 2, data_buffer, 4);
        mem.operation(gp, delay);
        REQUIRE(gp.get_response_status() == tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
}
