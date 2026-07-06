#ifndef MASTER_MODULE_HPP
#define MASTER_MODULE_HPP

#include "utils.hpp"

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
    sc_master_module(sc_module_name name, sc_trace_file* tf)
        : sc_module(name) {

        sc_trace(tf, this->address, std::string(name) + "/address");
        sc_trace(tf, this->input, std::string(name) + "/input");
        sc_trace(tf, this->out, std::string(name) + "/out");
        sc_trace(tf, this->read, std::string(name) + "/read");
        sc_trace(tf, this->write, std::string(name) + "/write");
        sc_trace(tf, this->ready, std::string(name) + "/ready");
        sc_trace(tf, this->clk, std::string(name) + "/clk");
    }
};

#endif // MASTER_MODULE_HPP

#ifndef MASTER_MODULE_PORTBUNDLE_HPP
#define MASTER_MODULE_PORTBUNDLE_HPP

template <int A_WIDTH, int D_WIDTH>
struct sc_master_portBundle {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;


    sc_signal<addr_t> address;
    sc_signal<data_t> input;
    sc_signal<data_t> out;

    sc_signal<sc_logic> write;
    sc_signal<sc_logic> read;
    sc_signal<sc_logic> ready;

    sc_master_portBundle() {}

    void bind(
        sc_out<addr_t>& _address,
        sc_in<data_t>& _input,
        sc_out<data_t>& _out,
        sc_out<sc_logic>& _write,
        sc_out<sc_logic>& _read,
        sc_in<sc_logic>& _ready
    ) {
        _address(address);
        _input(input);
        _out(out);
        _write(write);
        _read(read);
        _ready(ready);

    }
};


#endif