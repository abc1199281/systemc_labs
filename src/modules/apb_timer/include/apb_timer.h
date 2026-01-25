#ifndef APB_TIMER_H
#define APB_TIMER_H

#include <systemc.h>

SC_MODULE(APB_TIMER) {
public:
    // --- 1. Ports ---
    sc_in<bool> clk;
    sc_in<bool> rst_n; // reset (Active Low)

    // APB Bus Interface
    sc_in<bool> PSEL;        // Device Select
    sc_in<bool> PENABLE;     // Transfer Enable
    sc_in<bool> PWRITE;      // 1=Write, 0=Read
    sc_in<sc_uint<32>> PADDR;  // Address
    sc_in<sc_uint<32>> PWDATA; // Write Data
    sc_out<sc_uint<32>> PRDATA;// Read Data

    // Interrupt Output
    sc_out<bool> irq;

    // --- 2. Register Map ---
    enum class RegOffsets: uint8_t {
        OFFSET_CTRL = 0x00, // Control Register
        OFFSET_LOAD = 0x04, // Reload Value Register
        OFFSET_VAL  = 0x08  // Current Value Register (Counter)
    };

    SC_CTOR(APB_TIMER);

    void run();     

    // --- 3. Internal Variables ---
    // In RTL, these are Flip-Flops
    sc_uint<32> r_val; // offset VAL
    // Getter for r_val for debugging
    sc_uint<32> get_r_val() const { return r_val; }

private:
    sc_uint<32> r_ctrl; // offset CTRL
    sc_uint<32> r_load; // offset LOAD

    void reset();
};

#endif