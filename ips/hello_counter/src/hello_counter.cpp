#include "hello_counter.h"

Hello_Counter::Hello_Counter(sc_module_name name) : sc_module(name) {
    // register process
    
    SC_METHOD(run_counter);
    sensitive << clk.pos(); 
    dont_initialize();      

    SC_METHOD(run_read);
    sensitive << clk.pos();
    dont_initialize();
    
    m_count = 0;
}

void Hello_Counter::run_counter() {    
    if (!rst_n.read()) {
        m_count = 0;
        return;
    }

    if (cmd_en.read() && cmd_we.read()) {
        if (addr.read() == 0x00) {
            m_count = wdata.read(); 
        }
    } 
    else {
        m_count++;
    }
}

void Hello_Counter::run_read() {
    if (!rst_n.read()) {
        rdata.write(0);
        return;
    }

    if (cmd_en.read() && !cmd_we.read()) {
        if (addr.read() == 0x00) {
            rdata.write(m_count);
        } else {
            rdata.write(0xDEADBEEF);
        }
    }
}