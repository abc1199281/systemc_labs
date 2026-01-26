#include "arbitrator.h"

arbitrator::arbitrator(sc_module_name name, int num_requestors)
    : sc_module(name), 
      req("req", num_requestors), 
      gnt("gnt", num_requestors),
      m_num_requestors(num_requestors) 
{
    SC_METHOD(arbitrate_method);
    for (int i = 0; i < m_num_requestors; ++i) {
        sensitive << req[i];
    }
    dont_initialize();
}

void arbitrator::arbitrate_method() {
    bool granted = false;
    
    // Default: clear all grants first
    for(int i=0; i<m_num_requestors; ++i) {
        gnt[i].write(false);
    }

    // Simple Fixed Priority: Lower index has higher priority
    for (int i = 0; i < m_num_requestors; ++i) {
        if (req[i].read() == true) {
            if (!granted) {
                gnt[i].write(true);
                granted = true;
                // Since this is fixed priority, once we grant the highest priority, we are done
                break; 
            }
        }
    }
}
