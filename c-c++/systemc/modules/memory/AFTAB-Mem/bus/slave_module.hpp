#ifndef SLAVE_MODULE_HPP
#define SLAVE_MODULE_HPP

#include "addrmap.hpp"
#include <memory>  // for std::unique_ptr
#include <map>
#include <string>
#include <utility>
#include <functional>  // std::reference_wrapper, std::ref

using namespace sc_dt;

template <int A_WIDTH, int D_WIDTH>
class sc_slave_module : public sc_module {
    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;
public:
    sc_in<bool> clk;
    sc_in<sc_logic> chipSelect;
    sc_in<sc_logic> read;
    sc_in<sc_logic> write;
    sc_in<addr_t> address;
    sc_in<data_t> input;
    sc_out<data_t> out;
    sc_out<sc_logic> ready;
    AddressMap* memory;

private:
    void setup_tracing(sc_trace_file* tf, sc_module_name name) {
        if (tf) {
            std::string prefix = std::string(name);
            sc_trace(tf, this->address, (prefix + "/address").c_str());
            sc_trace(tf, this->input, (prefix + "/input").c_str());
            sc_trace(tf, this->out, (prefix + "/out").c_str());
            sc_trace(tf, this->read, (prefix + "/read").c_str());
            sc_trace(tf, this->write, (prefix + "/write").c_str());
            sc_trace(tf, this->chipSelect, (prefix + "/chipSelect").c_str());
            sc_trace(tf, this->ready, (prefix + "/ready").c_str());
        }
    }

protected:

    sc_slave_module(sc_module_name name, sc_trace_file* tf,
        sc_address_space* space, int memorySize)
        : sc_module(name) {
        memory = new AddressMap(memorySize, space);
        setup_tracing(tf, name);
    }

    // for wrapper types
    sc_slave_module(sc_module_name name, sc_trace_file* tf)
        : sc_module(name), memory(nullptr) {
        setup_tracing(tf, name);
    }
};
#endif // SLAVE_MODULE_HPP
