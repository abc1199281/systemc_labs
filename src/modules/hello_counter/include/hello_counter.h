#ifndef HELLO_COUNTER_H
#define HELLO_COUNTER_H

#include <systemc.h>

SC_MODULE(Hello_Counter) {
    // --- Ports ---
    sc_in<bool> clk;          
    sc_in<bool> rst_n;        // reset (Active Low)
    
    // register interface
    sc_in<bool>        cmd_en;    // Enable (1 = Valid Command)
    sc_in<bool>        cmd_we;    // Write Enable (1 = Write, 0 = Read)
    sc_in<sc_uint<8>>  addr;     
    sc_in<sc_uint<32>> wdata;    
    sc_out<sc_uint<32>> rdata;   

public:
    // --- Internal Variables ---
    sc_uint<32> m_count;

    // --- Constructor & Processes ---
    SC_CTOR(Hello_Counter);  

    void run_counter();    
    void run_read();       
};

#endif