#ifndef MASTER_MODULE_HPP
#define MASTER_MODULE_HPP

#include "mempart.hpp"

template <int A_WIDTH, int D_WIDTH>
class sc_master_module : public sc_module {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

public:

    sc_in<bool> clk;

    sc_out<addr_t> address;
    sc_in<data_t> input;
    sc_out<data_t> out;

    sc_out<sc_logic> write;
    sc_out<sc_logic> read;
    sc_in<sc_logic> ready;

protected:
    sc_master_module(sc_module_name name, sc_trace_file* tf, sc_clock& _clk)
        : sc_module(name),
        clk(_clk) {
        std::string prefix = std::string(this->name());
        sc_trace(tf, this->address, prefix + "/address");
        sc_trace(tf, this->input, prefix + "/input");
        sc_trace(tf, this->out, prefix + "/out");
        sc_trace(tf, this->read, prefix + "/read");
        sc_trace(tf, this->write, prefix + "/write");
        sc_trace(tf, this->ready, prefix + "/ready");
    }
};

#endif // MASTER_MODULE_HPP