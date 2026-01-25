#include "apb_timer.h"

APB_TIMER::APB_TIMER(sc_module_name name) : sc_module(name) {
    // register process
    
    SC_METHOD(run);
    sensitive << clk.pos(); 
    dont_initialize();      
    async_reset_signal_is(rst_n, false);
    reset();
}

void APB_TIMER::reset() {  
    r_ctrl = 0;
    r_load = 0;
    r_val = 0;
}

void APB_TIMER::run() {    
    // 1. Synchronous Reset
    if (!rst_n.read()) {
        reset();
        irq.write(false);  
        PRDATA.write(0);
        return;
    }

    // 2. APB Bus Write Logic
    if (PSEL.read() && PENABLE.read() && PWRITE.read()) {
        sc_uint<32> addr = PADDR.read() & 0xFF;
        switch (static_cast<RegOffsets>(addr.to_uint())) {
            case RegOffsets::OFFSET_CTRL:
                r_ctrl = PWDATA.read();
                break;
            case RegOffsets::OFFSET_LOAD:
                r_load = PWDATA.read();
                r_val  = PWDATA.read();
                break;
            case RegOffsets::OFFSET_VAL:
                // Read-Only Register; ignore writes
                break;
            default:
                // Invalid address; ignore
                break;
        }
    }

    // 3. APB Bus Read Logic
    if (PSEL.read() && PENABLE.read() && !PWRITE.read()) {
        sc_uint<32> addr = PADDR.read() & 0xFF;
        switch (static_cast<RegOffsets>(addr.to_uint())) {
            case RegOffsets::OFFSET_CTRL:
                PRDATA.write(r_ctrl);
                break;
            case RegOffsets::OFFSET_LOAD:
                PRDATA.write(r_load);
                break;
            case RegOffsets::OFFSET_VAL:
                PRDATA.write(r_val);
                break;
            default:
                PRDATA.write(0);
                break;
        }
    }else{
        PRDATA.write(0);    
    }

    std::cout << sc_time_stamp() << " APB_TIMER::run() Pre-Core Logic - r_ctrl: " << r_ctrl << ", r_val: " << r_val << ", irq: " << irq.read() << std::endl;
    // 4. Core Logic
    if (r_ctrl & 0x1) { 
        if (r_val == 0) {
            // Underflow Event!
            // 1. Reload counter from Load Register
            r_val = r_load;
            // 2. Fire Interrupt (Pulse High)
            irq.write(true);
        } else {
            // Countdown
            r_val = r_val - 1;
            // Ensure IRQ is false during normal counting
            irq.write(false);
        }
    } else {
        // Timer Paused
        irq.write(false);
    }
    std::cout << sc_time_stamp() << " APB_TIMER::run() After-Core Logic - r_ctrl: " << r_ctrl << ", r_val: " << r_val << ", irq: " << irq.read() << std::endl;
}