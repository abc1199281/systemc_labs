#ifndef UART_TX_H
#define UART_TX_H

#include <systemc.h>

SC_MODULE(UART_TX) {
public:
    // --- Ports ---
    sc_in<bool> clk;
    sc_in<bool> rst_n; // reset (Active Low)

    // register interface
    sc_in<bool> cmd_en;    // Enable (1 = Valid Command)
    sc_in<bool> cmd_we;    // Write Enable (1 = Write, 0 = n.o.)
    sc_in<sc_uint<8>> data_in;    
    sc_out<bool> data_out;   
    sc_out<bool> tx_busy;

    // --- Internal Variables ---
    enum class State : uint8_t {
        s_IDLE,
        s_START,
        s_TX_DATA,
        s_STOP
    };
    sc_uint<32> baud_period; // assigned when construction
    sc_uint<32> s_count; // counts the state machine bit periods
    sc_uint<32> baud_tick_count; // counts the baud ticks within a bit period    
    sc_uint<8> tx_shift_reg; // shift register for transmitting data
    State m_state;

    SC_CTOR(UART_TX);

    void run_tx();     

    void reset();
};

#endif