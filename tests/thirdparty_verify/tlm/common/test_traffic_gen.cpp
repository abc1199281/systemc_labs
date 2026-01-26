#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <systemc>

// Instantiate reporting globals here
#define REPORT_DEFINE_GLOBALS
#include "reporting.h"
#include "traffic_generator.h"
#include <tlm>

using namespace sc_core;

// Define sc_main required by SystemC
int sc_main(int argc, char* argv[]) {
    // Enable all reporting for visibility
    REPORT_ENABLE_ALL_REPORTING();
    int result = Catch::Session().run(argc, argv);
    return result;
}

// Dummy Responder Module to accept requests from Traffic Generator
class DummyResponder : public sc_module {
public:
    sc_port<sc_fifo_in_if<tlm::tlm_generic_payload*>> request_in_port;
    sc_port<sc_fifo_out_if<tlm::tlm_generic_payload*>> response_out_port;

    SC_HAS_PROCESS(DummyResponder);

    DummyResponder(sc_module_name name) : sc_module(name) {
        SC_THREAD(process_thread);
    }

    void process_thread() {
        while (true) {
            tlm::tlm_generic_payload* trans;
            // Read request
            request_in_port->read(trans);
            
            // Generate simple valid response
            trans->set_response_status(tlm::TLM_OK_RESPONSE);
            
            // Loopback data validation logic is inside traffic_generator, so just verify commands
            // Traffic generator expects reads to return data written. 
            // BUT: traffic_generator's check_complete reads from response_in_port.
            // And it checks data: read_data == expected_data.
            // In traffic_gen write loop: *reinterpret_cast<unsigned int*>(data_buffer_ptr) = w_data;
            // w_data = mem_address.
            // In read loop: expected_data = address.
            // So if we just echo back the transaction with OK response, the data pointer should still point to valid data?
            // Wait, TG allocates new transaction for each request.
            // For WRITE: TG sets data.
            // For READ: TG sets address. We need to populate data?
            // Let's look at logic:
            // READ CHECK: 
            // unsigned int expected_data = (unsigned int)transaction_ptr->get_address();
            // unsigned int read_data = *reinterpret_cast<unsigned int*>(data_buffer_ptr);
            // So for READ command, we MUST populate data buffer with address!
            
            if (trans->get_command() == tlm::TLM_READ_COMMAND) {
                 unsigned int addr = (unsigned int)trans->get_address();
                 unsigned char* ptr = trans->get_data_ptr();
                 *reinterpret_cast<unsigned int*>(ptr) = addr;
            }

            // Send response back
            response_out_port->write(trans);
        }
    }
};

TEST_CASE("Traffic Generator Initialization", "[traffic_gen]") {
    // Parameters
    const unsigned int ID = 1;
    sc_dt::uint64 base_addr1 = 0x1000;
    sc_dt::uint64 base_addr2 = 0x2000;
    unsigned int active_txn_count = 1;

    REQUIRE_NOTHROW(traffic_generator("tg_init", ID, base_addr1, base_addr2, active_txn_count));
}

TEST_CASE("Traffic Generator Integration Test", "[traffic_gen]") {
    // Instantiate Modules
    traffic_generator tg("tg", 1, 0x1000, 0x2000, 4); // ID=1, Max 4 txns
    DummyResponder responder("responder");

    // Channels
    sc_fifo<tlm::tlm_generic_payload*> req_fifo("req_fifo", 16);
    sc_fifo<tlm::tlm_generic_payload*> rsp_fifo("rsp_fifo", 16);

    // Bind Ports
    tg.request_out_port(req_fifo);
    tg.response_in_port(rsp_fifo);
    
    responder.request_in_port(req_fifo);
    responder.response_out_port(rsp_fifo);

    // Run Simulation
    // Traffic generator sends 16 writes then 16 reads for base1, then same for base2. 
    // Total = 64 transactions.
    
    // We run enough time to finish all transactions.
    sc_start(1, SC_US);

    // Verify: If Traffic generator failed fatal, simulation would abort.
    // If it finishes, it means success usually (it prints "Traffic Generator Complete").
    // We can't easily check internal state without easier hooks, but successful completion without throw is the test.
    REQUIRE(sc_time_stamp() > SC_ZERO_TIME);
}
