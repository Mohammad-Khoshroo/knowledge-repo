#include "head.hpp"
#include "sc_report_peronalized.hpp"

// time scale = 1NS/1NS
template<int clkHalfPeriod = 10>
class TB : public sc_module {
private:
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> rst;

    // Clock Generator
    void clocking() {

        clk.write(SC_LOGIC_0);
        while (true) {

            clk.write(SC_LOGIC_1);
            wait(clkHalfPeriod, SC_NS);
            clk.write(SC_LOGIC_0);
            wait(clkHalfPeriod, SC_NS);
        }
    };

    // Restart System
    void restarting() {

        rst.write(SC_LOGIC_1);
        wait(10, SC_NS);
        rst.write(SC_LOGIC_0);
        wait(10, SC_NS);
    };

public:
    // Constructor 
    TB(sc_module_name name, sc_trace_file* tf) : sc_module(name) {


        sc_trace(tf, clk, "clk");
        sc_trace(tf, rst, "rst");

        SC_THREAD(clocking);
        SC_THREAD(restarting);
    }


};

int sc_main() {

    // personal report handler
    sc_report_personalized::init_report_handler("_tb.log");

    sc_trace_file* tf = sc_create_vcd_trace_file("_tb");
    tf->set_time_unit(1, SC_NS);

    // Encapsulating trace file handling by passing it as a constructor argument to each class
    // Sharing a single trace file across classes by passing it as a reference to support encapsulation
    TB<>* testBench = new TB<>("_tb", tf);

    sc_close_vcd_trace_file(tf);
    sc_report_personalized::close_report_log();

    return 0;
}