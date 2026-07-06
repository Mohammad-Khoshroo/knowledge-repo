#ifndef MASTER_MODULE_PORTBUNDLE_HPP
#define MASTER_MODULE_PORTBUNDLE_HPP

#include "master_module.hpp"

using namespace sc_dt;

template <int A_WIDTH, int D_WIDTH>
struct sc_master_portBundle {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

    sc_signal<data_t> input;

    sc_signal<addr_t> address;
    sc_signal<data_t> out;

    sc_signal<sc_logic> ready;
    sc_signal<sc_logic> write;
    sc_signal<sc_logic> read;

    sc_master_portBundle() = default;

    void bind(
        sc_out<addr_t>& address_,
        sc_out<data_t>& out_,
        sc_in<data_t>& input_,
        sc_out<sc_logic>& write_,
        sc_out<sc_logic>& read_,
        sc_in<sc_logic>& ready_
    ) {
        address_(address);
        input_(input);
        out_(out);
        write_(write);
        read_(read);
        ready_(ready);

    }

    void initialize() {
        input = 0;
        ready = SC_LOGIC_0;
        write = SC_LOGIC_0;
        read = SC_LOGIC_0;

    }

    void trace(sc_trace_file* tf, std::string& prefix) {
        sc_trace(tf, this->address, (prefix + "/address").c_str());
        sc_trace(tf, this->input, (prefix + "/input").c_str());
        sc_trace(tf, this->out, (prefix + "/out").c_str());
        sc_trace(tf, this->ready, (prefix + "/ready").c_str());
        sc_trace(tf, this->read, (prefix + "/read").c_str());
        sc_trace(tf, this->write, (prefix + "/write").c_str());
    }

};

#endif