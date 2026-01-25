#include "uart_tx.h"

UART_TX::UART_TX(sc_module_name name) : sc_module(name) {
    // register process
    
    SC_METHOD(run_tx);
    sensitive << clk.pos(); 
    dont_initialize();      
    reset();
}

void UART_TX::reset() {    
    baud_period = 868; // for 115200 baud with 100MHz clock
    s_count = 0;
    baud_tick_count = 0;
    tx_shift_reg = 0;
    m_state = State::s_IDLE;
}

void UART_TX::run_tx() {    
    if (!rst_n.read()) {
        reset();
        data_out.write(true); // output signals can only be set here
        tx_busy.write(false);
        return;
    }

    baud_tick_count++;

    switch(m_state){
        case State::s_IDLE:
            if (cmd_en.read() && cmd_we.read()) {
                m_state = State::s_START;
                s_count = 0;
                baud_tick_count = 0;
                tx_shift_reg = data_in.read();
                tx_busy.write(true);
                data_out.write(false);
            }
            break;
        case State::s_START:
            if (baud_tick_count == baud_period) {
                m_state = State::s_TX_DATA;
                baud_tick_count = 0;
                data_out.write(tx_shift_reg[0]);
                tx_shift_reg = tx_shift_reg >> 1;
                s_count = 1;
            }
            break;
        case State::s_TX_DATA:
            if (baud_tick_count == baud_period) {
                if (s_count == 8) {
                    m_state = State::s_STOP;
                    data_out.write(true); // stop bit
                    s_count = 0;
                } else {
                    s_count++;
                    data_out.write(tx_shift_reg[0]);
                    tx_shift_reg = tx_shift_reg >> 1;
                }      
                baud_tick_count = 0;         
            }
            break;
        case State::s_STOP:
            if (baud_tick_count == baud_period) {
                m_state = State::s_IDLE;
                data_out.write(true); // idle state
                tx_busy.write(false);
                baud_tick_count = 0;
            }
            break;
        default:
            m_state = State::s_IDLE;
            break;  
    }
}