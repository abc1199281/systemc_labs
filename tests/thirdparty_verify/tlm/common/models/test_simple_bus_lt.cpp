#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <systemc>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"

// The reporting globals need to be defined in one compilation unit
#define REPORT_DEFINE_GLOBALS
#include "reporting.h"
#include "models/SimpleBusLT.h"
#include "memory.h"

using namespace sc_core;
using namespace tlm;

// Define sc_main which is required by SystemC
int sc_main(int argc, char* argv[]) {
    // Initialize reporting enables
    REPORT_ENABLE_ALL_REPORTING();
    
    int result = Catch::Session().run(argc, argv);
    return result;
}

// Simple Initiator module for testing the bus
class SimpleInitiator : public sc_module {
public:
    tlm_utils::simple_initiator_socket<SimpleInitiator> socket;

    SC_CTOR(SimpleInitiator) : socket("socket") {}

    void write(uint64_t addr, unsigned char* data, unsigned int len) {
        tlm_generic_payload gp;
        sc_time delay = SC_ZERO_TIME;
        gp.set_command(TLM_WRITE_COMMAND);
        gp.set_address(addr);
        gp.set_data_ptr(data);
        gp.set_data_length(len);
        gp.set_streaming_width(len);
        gp.set_byte_enable_ptr(0);
        gp.set_dmi_allowed(false);
        gp.set_response_status(TLM_INCOMPLETE_RESPONSE);

        socket->b_transport(gp, delay);
        REQUIRE(gp.get_response_status() == TLM_OK_RESPONSE);
    }

    void read(uint64_t addr, unsigned char* data, unsigned int len) {
        tlm_generic_payload gp;
        sc_time delay = SC_ZERO_TIME;
        gp.set_command(TLM_READ_COMMAND);
        gp.set_address(addr);
        gp.set_data_ptr(data);
        gp.set_data_length(len);
        gp.set_streaming_width(len);
        gp.set_byte_enable_ptr(0);
        gp.set_dmi_allowed(false);
        gp.set_response_status(TLM_INCOMPLETE_RESPONSE);

        socket->b_transport(gp, delay);
        REQUIRE(gp.get_response_status() == TLM_OK_RESPONSE);
    }
};

// Simple Target module that wraps the memory class
class SimpleTarget : public sc_module {
public:
    tlm_utils::simple_target_socket<SimpleTarget> socket;

    SimpleTarget(sc_module_name name, uint32_t id, uint64_t size)
        : sc_module(name)
        , socket("socket")
        , m_mem(id, SC_ZERO_TIME, SC_ZERO_TIME, size, 4) 
    {
        socket.register_b_transport(this, &SimpleTarget::b_transport);
    }

    void b_transport(tlm_generic_payload& gp, sc_time& delay) {
        m_mem.operation(gp, delay);
    }

    unsigned char* get_mem_ptr() { return m_mem.get_mem_ptr(); }

private:
    memory m_mem;
};

TEST_CASE("SimpleBusLT Routing and Forwarding", "[SimpleBusLT]") {
    SimpleInitiator initiator("initiator");
    SimpleBusLT<1, 2> bus("bus");
    SimpleTarget target0("target0", 0, 1024);
    SimpleTarget target1("target1", 1, 1024);

    // Bindings
    initiator.socket.bind(bus.target_socket[0]);
    bus.initiator_socket[0].bind(target0.socket);
    bus.initiator_socket[1].bind(target1.socket);

    SECTION("Route to Target 0") {
        unsigned char data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        // Base address 0x0... maps to port 0
        uint64_t addr = 0x00000010;
        initiator.write(addr, data, 4);

        unsigned char read_data[4] = {0};
        initiator.read(addr, read_data, 4);
        
        for(int i=0; i<4; i++) {
            REQUIRE(read_data[i] == data[i]);
        }
    }

    SECTION("Route to Target 1") {
        unsigned char data[] = {0x11, 0x22, 0x33, 0x44};
        // Base address 0x1... maps to port 1
        uint64_t addr = 0x10000020;
        initiator.write(addr, data, 4);

        unsigned char read_data[4] = {0};
        initiator.read(addr, read_data, 4);
        
        for(int i=0; i<4; i++) {
            REQUIRE(read_data[i] == data[i]);
        }
    }

    SECTION("Address Masking Check") {
        unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF};
        uint64_t addr = 0x10000050; // Points to target 1
        initiator.write(addr, data, 4);

        // Verify backdoor that address was masked to 0x50 in target 1
        unsigned char* mem_ptr = target1.get_mem_ptr();
        for(int i=0; i<4; i++) {
            REQUIRE(mem_ptr[0x50 + i] == data[i]);
        }
    }
}
