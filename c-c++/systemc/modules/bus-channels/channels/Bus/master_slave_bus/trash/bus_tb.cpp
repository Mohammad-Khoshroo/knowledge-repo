#include "bus.hpp"

#define Address_WIDTH 8
#define Data_WIDTH 8
#define MASTERS 1
#define SLAVES 2

using namespace std;


template <int A_WIDTH, int D_WIDTH>
class dummy_slave : public sc_slave_module<A_WIDTH, D_WIDTH> {
    using data_t = sc_lv<D_WIDTH>;
    using addr_t = sc_lv<A_WIDTH>;

public:
    SC_HAS_PROCESS(dummy_slave);
    dummy_slave(sc_module_name name, sc_trace_file* tf, int startAddr, int size)
        : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, startAddr, size) {

        SC_METHOD(respond);
        this->sensitive << this->clk.pos();
    }

    void respond() {
        if (this->chipSelect.read() == SC_LOGIC_1) {
            if (this->write.read() == SC_LOGIC_1) {
                mem[this->address.read().to_uint()] = this->input.read();
                this->ready.write(SC_LOGIC_1);
            }
            else if (this->read.read() == SC_LOGIC_1) {
                this->out.write(mem[this->address.read().to_uint()]);
                this->ready.write(SC_LOGIC_1);
            }
            else {
                this->ready.write(SC_LOGIC_0);
            }
        }
        else {
            this->ready.write(SC_LOGIC_0);
        }
    }

private:
    data_t mem[256] = {};
};
// رنگ‌ها برای خروجی
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_RED     "\033[31m"

template <int A_WIDTH, int D_WIDTH>
class dummy_master : public sc_master_module<A_WIDTH, D_WIDTH> {
    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

public:
    SC_HAS_PROCESS(dummy_master);
    dummy_master(sc_module_name name, sc_trace_file* tf)
        : sc_master_module<A_WIDTH, D_WIDTH>(name, tf) {

        SC_THREAD(main);
        this->sensitive << this->clk.pos();
    }

    void main() {
        wait(5); // صبر اولیه

        for (int i = 0; i < 4; ++i) {
            addr_t addr = addr_t(i + 0x10);
            data_t value = data_t(0xA0 + i);

            std::cout << COLOR_GREEN << "[MASTER] Writing 0x"
                      << std::hex << value.to_uint() << " to address 0x"
                      << addr.to_uint() << COLOR_RESET << std::endl;

            this->address.write(addr);
            this->out.write(value);
            this->write.write(SC_LOGIC_1);
            wait();
            this->write.write(SC_LOGIC_0);

            while (this->ready.read() != SC_LOGIC_1) wait();

            wait(); // فاصله بین عملیات
        }

        wait(2);

        for (int i = 0; i < 4; ++i) {
            addr_t addr = addr_t(i + 0x10);

            this->address.write(addr);
            this->read.write(SC_LOGIC_1);
            wait();
            this->read.write(SC_LOGIC_0);

            while (this->ready.read() != SC_LOGIC_1) wait();

            data_t result = this->input.read();
            std::cout << COLOR_BLUE << "[MASTER] Read from 0x"
                      << std::hex << addr.to_uint() << ": 0x"
                      << result.to_uint() << " at " << sc_time_stamp()
                      << COLOR_RESET << std::endl;

            wait();
        }

        wait(2);
        sc_stop();
    }
};


// sc_main
int sc_main(int argc, char* argv[]) {

    sc_set_time_resolution(1, SC_NS);
    sc_trace_file* tf = sc_create_vcd_trace_file("bus_trace");

    sc_clock clk("clk", 2, SC_NS);






    sc_bus<MASTERS, SLAVES, Address_WIDTH, Data_WIDTH> bus("bus", tf);
    bus.clk(clk);

    dummy_master<Address_WIDTH, Data_WIDTH> m1("master1", tf);
    dummy_slave<Address_WIDTH, Data_WIDTH> s1("slave1", tf, 0x00, 0x10);  // 0x00 - 0x0F
    std::cout << "memory top" << MemoryPartition::top() << endl;

    dummy_slave<Address_WIDTH, Data_WIDTH> s2("slave2", tf, 16, 0x20);  // 0x10 - 0x2F

    std::cout << "memory top" << MemoryPartition::top() << endl;

    m1.clk(clk);
    s1.clk(clk);
    s2.clk(clk);
    // std::cout << "set clocks" << endl;
    
    bus.addMaster(&m1);
    // std::cout << "set master" << endl;
    
    bus.addSlave(&s1);
    bus.addSlave(&s2);
    // std::cout << "set slave" << endl;

    sc_start();
    sc_close_vcd_trace_file(tf);

    return 0;
}
