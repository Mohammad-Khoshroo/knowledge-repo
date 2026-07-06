#include "slave_module.hpp"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

template <int A_WIDTH, int D_WIDTH>
class sc_slave_module_impl : public sc_slave_module<A_WIDTH, D_WIDTH> {
public:
    SC_HAS_PROCESS(sc_slave_module_impl);

    sc_slave_module_impl(sc_module_name name, sc_trace_file* tf, int memStart, int memSize)
        : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, memStart, memSize)
    {
        SC_METHOD(process);
        this->sensitive << this->clk.pos(); // حساس به لبه بالارونده clk
    }

    void process() {
        if (this->chipSelect.read() == SC_LOGIC_1 && this->read.read() == SC_LOGIC_1) {
            this->out.write(this->input.read());
        }
        else {
            this->out.write(0);
        }
    }
};

SC_MODULE(Testbench) {
    sc_clock clk_sig{ "clk_sig", 10, SC_NS };
    sc_signal<sc_lv<8>> addr_sig;
    sc_signal<sc_lv<16>> data_in_sig;
    sc_signal<sc_lv<16>> data_out_sig;
    sc_signal<sc_logic> read_sig, write_sig, cs_sig, ready_sig;

    sc_trace_file* tf;
    sc_slave_module_impl<8, 16> slave;

    SC_CTOR(Testbench)
        : tf(sc_create_vcd_trace_file("slave_module_impl_test")),
        slave("slave", tf, 0, 256)
    {
        // Bind signals
        slave.clk(clk_sig);
        slave.address(addr_sig);
        slave.input(data_in_sig);
        slave.out(data_out_sig);
        slave.read(read_sig);
        slave.write(write_sig);
        slave.chipSelect(cs_sig);
        slave.ready(ready_sig);

        SC_THREAD(run);
    }

    void run() {
        cout << "\n[TEST] Scenario 1: chipSelect=1, read=1 => output should follow input" << endl;
        addr_sig.write("00000001");
        data_in_sig.write("1111000011110000");
        cs_sig.write(SC_LOGIC_1);
        read_sig.write(SC_LOGIC_1);
        write_sig.write(SC_LOGIC_0);
        ready_sig.write(SC_LOGIC_1);

        wait(15, SC_NS); // بیشتر از یک کلاک صبر کن

        cout << "    Output = " << data_out_sig.read().to_string() << endl;
        sc_assert(data_out_sig.read() == sc_lv<16>("1111000011110000"));
        cout << "    [PASS] Scenario 1\n" << endl;

        cout << "[TEST] Scenario 2: chipSelect=0, read=1 => output should be 0" << endl;
        cs_sig.write(SC_LOGIC_0);
        read_sig.write(SC_LOGIC_1);
        data_in_sig.write("1010101010101010");

        wait(10, SC_NS);

        cout << "    Output = " << data_out_sig.read().to_string() << endl;
        sc_assert(data_out_sig.read() == 0);
        cout << "    [PASS] Scenario 2\n" << endl;

        cout << "[TEST] Scenario 3: chipSelect=1, read=0 => output should be 0" << endl;
        cs_sig.write(SC_LOGIC_1);
        read_sig.write(SC_LOGIC_0);
        data_in_sig.write("1111111100000000");

        wait(10, SC_NS);

        cout << "    Output = " << data_out_sig.read().to_string() << endl;
        sc_assert(data_out_sig.read() == 0);
        cout << "    [PASS] Scenario 3\n" << endl;

        cout << "[TEST] Scenario 4: chipSelect=1, read=1, input changes => output changes on clk" << endl;
        cs_sig.write(SC_LOGIC_1);
        read_sig.write(SC_LOGIC_1);

        data_in_sig.write("0000111100001111");
        wait(10, SC_NS);
        cout << "    Output (step 1) = " << data_out_sig.read().to_string() << endl;
        sc_assert(data_out_sig.read() == sc_lv<16>("0000111100001111"));

        data_in_sig.write("1010101010101010");
        wait(10, SC_NS);
        cout << "    Output (step 2) = " << data_out_sig.read().to_string() << endl;
        sc_assert(data_out_sig.read() == sc_lv<16>("1010101010101010"));
        cout << "    [PASS] Scenario 4\n" << endl;

        // بستن فایل trace و پایان شبیه‌سازی
        sc_close_vcd_trace_file(tf);
        sc_stop();
    }

    ~Testbench() {
        // اطمینان از بستن فایل trace اگر هنوز باز است
        if (tf) {
            sc_close_vcd_trace_file(tf);
            tf = nullptr;
        }
    }
};

int sc_main(int argc, char* argv[]) {
    Testbench tb("tb");
    sc_start();
    return 0;
}
