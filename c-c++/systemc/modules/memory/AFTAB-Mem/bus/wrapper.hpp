#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include "bus/slave_module.hpp"
#include <systemc.h>

template <int A_WIDTH, int SLAVE_D_WIDTH, int BUS_D_WIDTH>
class sc_wrapper_module : public sc_slave_module<A_WIDTH, BUS_D_WIDTH> {

    using addr_t = sc_lv<A_WIDTH>;
    using slave_data_t = sc_lv<SLAVE_D_WIDTH>;
    using bus_data_t = sc_lv<BUS_D_WIDTH>;

public:

    // Internal signals for slave connection
    sc_signal<slave_data_t> slave_input;
    sc_signal<slave_data_t> slave_out;
    sc_signal<sc_logic> slave_ready;

    SC_HAS_PROCESS(sc_wrapper_module);
    sc_wrapper_module(sc_module_name name, sc_trace_file* tf,
        sc_slave_module<A_WIDTH, SLAVE_D_WIDTH>* slave)
        : sc_slave_module<A_WIDTH, BUS_D_WIDTH>(name, tf) {

        // set Address Mapping settings  
        this->memory = slave->memory;

        // Connect slave to wrapper's interface
        slave->clk(this->clk);
        slave->chipSelect(this->chipSelect);
        slave->read(this->read);
        slave->write(this->write);
        slave->address(this->address);
        slave->input(slave_input);
        slave->out(slave_out);
        slave->ready(slave_ready);

        // Register data conversion processes
        SC_METHOD(input_converter);
        this->sensitive << this->input;

        SC_METHOD(output_converter);
        this->sensitive << slave_out << slave_ready;
    }

private:

    void input_converter() {
        // Convert bus input to slave input
        if constexpr (BUS_D_WIDTH == SLAVE_D_WIDTH) {
            slave_input.write(this->input.read());
        }
        else if constexpr (BUS_D_WIDTH > SLAVE_D_WIDTH) {
            // Truncate wider bus data to slave width
            bus_data_t bus_data = this->input.read();
            slave_data_t slave_data;
            for (int i = 0; i < SLAVE_D_WIDTH; i++) {
                slave_data[i] = bus_data[i];
            }
            slave_input.write(slave_data);
        }
        else {
            // Zero-extend narrower bus data to slave width
            bus_data_t bus_data = this->input.read();
            slave_data_t slave_data = 0;
            for (int i = 0; i < BUS_D_WIDTH; i++) {
                slave_data[i] = bus_data[i];
            }
            slave_input.write(slave_data);
        }
    }

    void output_converter() {
        // Forward ready signal
        this->ready.write(slave_ready.read());

        // Convert slave output to bus output
        if constexpr (BUS_D_WIDTH == SLAVE_D_WIDTH) {
            this->out.write(slave_out.read());
        }
        else if constexpr (BUS_D_WIDTH > SLAVE_D_WIDTH) {
            // Zero-extend slave data to bus width
            slave_data_t slave_data = slave_out.read();
            bus_data_t bus_data = 0;
            for (int i = 0; i < SLAVE_D_WIDTH; i++) {
                bus_data[i] = slave_data[i];
            }
            this->out.write(bus_data);
        }
        else {
            // Truncate slave data to bus width
            slave_data_t slave_data = slave_out.read();
            bus_data_t bus_data;
            for (int i = 0; i < BUS_D_WIDTH; i++) {
                bus_data[i] = slave_data[i];
            }
            this->out.write(bus_data);
        }
    }
};

#endif // WRAPPER_HPP