#ifndef SLAVE_MODULE_HPP
#define SLAVE_MODULE_HPP

#include "mempart.hpp"

template <int A_WIDTH, int D_WIDTH>
class sc_slave_module : public sc_module {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

public:

    sc_in<bool> clk;

    sc_in<addr_t> address;
    sc_in<data_t> input;
    sc_out<data_t> out;

    sc_in<sc_logic> read;
    sc_in<sc_logic> write;
    sc_in<sc_logic> chipSelect;
    sc_out<sc_logic> ready;

    MemoryPartition memory;

protected:
    sc_slave_module(sc_module_name name, sc_trace_file* tf, sc_clock& _clk, int memoryStartAddr, int memorySize)
        : sc_module(name),
        clk(_clk),
        memory(memoryStartAddr, memorySize) {

        sc_trace(tf, this->address, std::string(this->name()) + "/address");
        sc_trace(tf, this->input, std::string(this->name()) + "/input");
        sc_trace(tf, this->out, std::string(this->name()) + "/out");
        sc_trace(tf, this->read, std::string(this->name()) + "/read");
        sc_trace(tf, this->write, std::string(this->name()) + "/write");
        sc_trace(tf, this->chipSelect, std::string(this->name()) + "/chipSelect");
        sc_trace(tf, this->ready, std::string(this->name()) + "/ready");
    }
};

#endif // SLAVE_MODULE_HPP
