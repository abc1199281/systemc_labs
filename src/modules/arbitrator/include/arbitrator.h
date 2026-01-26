#ifndef ARBITRATOR_H
#define ARBITRATOR_H

#include <systemc.h>

class arbitrator : public sc_module {
public:
    // Port declarations
    // Support scalable number of requestors using sc_vector
    sc_vector<sc_in<bool>> req;
    sc_vector<sc_out<bool>> gnt;

    SC_HAS_PROCESS(arbitrator);

    arbitrator(sc_module_name name, int num_requestors);

private:
    int m_num_requestors;
    void arbitrate_method();
};

#endif // ARBITRATOR_H
