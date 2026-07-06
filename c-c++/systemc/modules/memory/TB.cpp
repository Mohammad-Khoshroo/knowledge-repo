#include "memory.hpp" // your module header
#include "sc_report_peronalized.hpp"
// Define the data type T here. Let's say T = sc_lv<WIDTH> for simplicity
// or use uint32_t for a 32-bit memory, depends on your T.

constexpr int WIDTH = 8; // for example
using T = sc_lv<WIDTH>;

SC_MODULE(MemoryTB)
{
    // Signals
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> chipSelect;
    sc_signal<sc_logic> read;
    sc_signal<sc_logic> write;
    sc_signal<sc_lv<WIDTH>> address;
    sc_signal<sc_lv<WIDTH>> input;
    sc_signal<sc_lv<WIDTH>> output;
    sc_signal<sc_logic> ready;
    sc_signal<sc_lv<3>> traceState;

    // dump times for memory dump thread
    std::vector<int> dumpTimes{0, 10, 20, 30};

    // Instantiate memory
    Memory<T, WIDTH> *mem;

    // Clock generator
    void clkGen()
    {
        while (true)
        {
            clk.write(SC_LOGIC_0);
            wait(5, SC_NS);
            clk.write(SC_LOGIC_1);
            wait(5, SC_NS);
        }
    }

    // Test procedure
    void test()
    {
        // Reset signals
        chipSelect.write(SC_LOGIC_0);
        read.write(SC_LOGIC_0);
        write.write(SC_LOGIC_0);
        address.write(0);
        input.write(0);
        wait(10, SC_NS);

        chipSelect.write(SC_LOGIC_1);

        // Write some data
        for (int i = 0; i < 10; i++)
        {
            address.write(i);
            input.write(sc_lv<WIDTH>(i + 1));
            write.write(SC_LOGIC_1);
            read.write(SC_LOGIC_0);
            wait(10, SC_NS);
            write.write(SC_LOGIC_0);
            wait(10, SC_NS);
        }

        // Read back data
        for (int i = 0; i < 10; i++)
        {
            address.write(i);
            write.write(SC_LOGIC_0);
            read.write(SC_LOGIC_1);
            wait(10, SC_NS);

            std::cout << "At time " << sc_time_stamp()
                      << " read address " << i
                      << " data = " << output.read().to_string() << std::endl;

            read.write(SC_LOGIC_0);
            wait(10, SC_NS);
        }

        // Test out of bounds read
        address.write(sc_lv<WIDTH>(std::string(5, '1').c_str())); // max address (all ones)
        read.write(SC_LOGIC_1);
        write.write(SC_LOGIC_0);
        wait(10, SC_NS);

        read.write(SC_LOGIC_0);
        wait(10, SC_NS);

        sc_stop(); // end simulation
    }

    SC_CTOR(MemoryTB)
    {
        mem = new Memory<T, WIDTH>("MemoryInstance", "_memory_input.csv", "_memory_dump.csv", dumpTimes);
        cout << "Memory instance created." << endl;
        mem->clk(clk);
        mem->chipSelect(chipSelect);
        mem->write(write);
        mem->read(read);
        mem->address(address);
        mem->input(input);
        mem->output(output);
        mem->ready(ready);
        mem->traceState(traceState);
        SC_THREAD(clkGen);
        SC_THREAD(test);
    }

    ~MemoryTB()
    {
        delete mem;
    }
};

int sc_main(int argc, char *argv[])
{
    sc_report_personalized::init_report_handler("TB.log");

    MemoryTB tb("tb");
    sc_start();
    sc_report_personalized::close_report_log();
    return 0;
}
