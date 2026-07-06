#include "utils.hpp"

template <int A_WIDTH, int D_WIDTH>
class sc_master_module : public sc_module, public PortTypes {
public:

    sc_in<sc_logic> clk;

    sc_out<addr_t> address;
    sc_in<data_t> input;
    sc_out<data_t> out;

    sc_out<sc_logic> write;
    sc_out<sc_logic> read;
    sc_in<sc_logic> ready;

protected:
    sc_master_module(sc_module_name name, sc_trace_file* tf)
        : sc_module(name) {
        // optional trace connections
    }
};

template <int A_WIDTH, int D_WIDTH>
struct sc_master_portBundle {
    sc_in<addr_t> address;
    sc_out<data_t> input;
    sc_in<data_t> out;

    sc_in<sc_logic> write;
    sc_in<sc_logic> read;
    sc_out<sc_logic> ready;

    sc_master_portBundle(
        sc_in<addr_t> address,
        sc_out<data_t> input,
        sc_in<data_t> out,
        sc_out<sc_logic> write,
        sc_out<sc_logic> read,
        sc_in<sc_logic> ready
    ) :
        address(address),
        input(input),
        out(out),
        write(write),
        read(read),
        ready(ready)
    {
    }
};