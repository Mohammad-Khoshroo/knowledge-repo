#include "utils.hpp"

template <int A_WIDTH, int D_WIDTH>
class sc_slave_module : public sc_module, public PortTypes {
public:

    sc_in<sc_logic> clk;

    sc_in<addr_t> address;
    sc_in<data_t> input;
    sc_out<data_t> out;

    sc_out<sc_logic> read;
    sc_out<sc_logic> write;
    sc_out<sc_logic> chipSelect;
    sc_in<sc_logic> ready;
    MemoryPartition memory;

protected:
    sc_slave_module(sc_module_name name, sc_trace_file* tf, int memoryStartAddr, int memorySize)
        : sc_module(name),
        memory(memoryStartAddr, memorySize) {
        // optional trace connections
    }
};

template <int A_WIDTH, int D_WIDTH>
struct sc_slave_portBundle {
    sc_out<addr_t> address;
    sc_out<data_t> input;
    sc_in<data_t> out;
    sc_out<sc_logic> read;
    sc_out<sc_logic> write;
    sc_out<sc_logic> chipSelect;
    sc_in<sc_logic> ready;

    sc_slave_portBundle(
        sc_out<addr_t> address,
        sc_out<data_t> input,
        sc_in<data_t> out,
        sc_out<sc_logic> read,
        sc_out<sc_logic> write,
        sc_out<sc_logic> chipSelect,
        sc_in<sc_logic> ready
    ) :
        address(address),
        input(input),
        out(out),
        read(read),
        write(write),
        chipSelect(chipSelect),
        ready(ready)
    {
    }
};