#include <systemc.h>

SC_MODULE(salam) {
    sc_signal<sc_lv<4>> my_signal;

    SC_CTOR(salam) {
        SC_METHOD(hi);
        sensitive << my_signal;

        SC_THREAD(test_driver);
    }

    void hi() {
        std::cout << "Current value: " << my_signal.read() << std::endl;
    }

    void test_driver() {
        wait(1, SC_NS);
        my_signal = 4;      // 0100
        wait(SC_ZERO_TIME);
        // sc_stop();
        // wait(1, SC_NS);
        // my_signal = sc_lv<4>("1010"); // 1010
        // wait(1, SC_NS);
        // my_signal = 0;                // 0000
    }
};

int sc_main(int, char* []) {
    salam x("salam");
    sc_start(10, SC_NS);
    return 0;
}
