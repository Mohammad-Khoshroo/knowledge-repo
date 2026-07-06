#include "DMA.hpp"
#include "bus.hpp"

#define Address_WIDTH 16
#define Data_WIDTH 16


template <int A_WIDTH, int D_WIDTH>
class addAccelerator : public sc_slave_module<A_WIDTH, D_WIDTH> {

    using data_t = sc_lv<D_WIDTH>;
    using addr_t = sc_lv<A_WIDTH>;

public:

    SC_HAS_PROCESS(addAccelerator);
    addAccelerator(sc_module_name name, sc_trace_file* tf, sc_clock& _clk, int startAddr, int size)
        : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, _clk, startAddr, size) {

        SC_THREAD(respond);
        this->sensitive << this->clk.pos();
    }

    void respond() {
        while (true)
        {
            if (this->chipSelect.read() == SC_LOGIC_1)
            {
                if (this->write.read() == SC_LOGIC_1) {
                    localReg[this->address.read().to_uint()] = this->input.read().to_int64();
                    this->ready = SC_LOGIC_1;
                }
                else if (this->read.read() == SC_LOGIC_1) {
                    localReg[3] = localReg[0].to_int64() + localReg[1].to_int64() + localReg[2].to_int64();
                    this->out = localReg[0].to_int64() + localReg[1].to_int64() + localReg[2].to_int64();
                    this->ready = SC_LOGIC_1;
                }
                else
                    this->ready = SC_LOGIC_0;
            }
            else
            {
                this->out = 0;
                this->ready = SC_LOGIC_0;
            }

            wait(this->clk.posedge_event());
        }
    }

private:
    data_t localReg[4] = { 0,0,0,0 };
};

// template <int A_WIDTH, int D_WIDTH>
// class MultAccelerator : public sc_slave_module<A_WIDTH, D_WIDTH> {

//     using data_t = sc_lv<D_WIDTH>;
//     using addr_t = sc_lv<A_WIDTH>;

// public:

//     SC_HAS_PROCESS(MultAccelerator);
//     MultAccelerator(sc_module_name name, sc_trace_file* tf, int startAddr, int size)
//         : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, startAddr, size) {

//         SC_THREAD(respond);
//         this->sensitive << this->clk.pos();
//     }

//     void respond() {

//         if (this->chipSelect.read() == SC_LOGIC_1)
//         {
//             if (this->write.read() == SC_LOGIC_1) {
//                 localReg[this->address.read().to_uint()] = this->input.read().to_int64();
//                 this->ready = SC_LOGIC_1;
//             }
//             else if (this->read.read() == SC_LOGIC_1) {
//                 this->out = localReg[0].to_int64() * localReg[1].to_int64() * localReg[2].to_int64();
//                 this->ready = SC_LOGIC_1;
//             }
//             else
//                 this->ready = SC_LOGIC_0;
//         }
//         else
//         {
//             this->out = 0;
//             this->ready = SC_LOGIC_0;
//         }

//         wait(SC_ZERO_TIME);
//     }

// private:
//     data_t localReg[3] = { 0,0,0 };
// };


template <int WIDTH>
class Memory : public sc_slave_module<WIDTH, WIDTH>
{
private:
    int memory[10] = { 3,4,7,1,5,4,3,2,4,10 };

    // when you read from Memory one Data
    void readMem();
    // when you write on Memory one Data
    void writeMem();
    // issue ready signal for handShaking
    void setMemReady();
    // checking conflict of read and write
    void checkConflict();

public:
    SC_HAS_PROCESS(Memory);
    Memory(sc_module_name name, sc_trace_file* tf, sc_clock& _clk, int startaddress) :
        sc_slave_module<WIDTH, WIDTH>(name, tf, _clk, startaddress, 10)

    {
        SC_METHOD(checkConflict);
        this->sensitive << this->read << this->write;

        // Asynchronous read
        SC_METHOD(readMem);
        this->sensitive << this->address << this->chipSelect << this->read;

        // synchronous write
        SC_THREAD(writeMem);
        this->sensitive << this->clk;

        SC_THREAD(setMemReady);
        this->sensitive << this->address << this->chipSelect << this->read << this->write;

    }
};
template <int WIDTH>
void Memory<WIDTH>::readMem()
{
    if ((this->chipSelect.read() == SC_LOGIC_1) && (this->read.read() == SC_LOGIC_1))
    {
        uint64_t addr = this->address.read().to_uint64();
        if (addr >= 10)
        {
            SC_REPORT_ERROR("Memory", "Read address out of bounds");
            std::string x = std::string(WIDTH, 'X');
            this->out.write(x.c_str());
            return; // Skip this read
        }
        else if (addr < 10)
            this->out = memory[addr];

        else
        {
            SC_REPORT_WARNING("Memory", "Read address exceeds memory size, returning X");
            std::string x = std::string(WIDTH, 'X');
            this->out.write(x.c_str());
        }
    }
}
template <int WIDTH>
void Memory<WIDTH>::writeMem()
{
    while (true)
    {
        if ((this->chipSelect.read() == SC_LOGIC_1) && (this->write.read() == SC_LOGIC_1))
        {

            uint64_t addr = this->address.read().to_uint64();
            if (addr >= 10)
            {
                SC_REPORT_WARNING("Memory", "Write address invalid");
                wait(this->clk.posedge_event()); // avoid tight loop busy wait
                continue;
            }

            int input = this->input.read().to_int();
            memory[addr] = input;
        }

        wait(this->clk.posedge_event());
    }
}
template <int WIDTH>
void Memory<WIDTH>::setMemReady()
{
    while (true)
    {
        this->ready.write(SC_LOGIC_0);

        if ((this->chipSelect.read() == SC_LOGIC_1) && ((this->write.read() == SC_LOGIC_1) || (this->read.read() == SC_LOGIC_1)))
        {
            uint64_t addr = this->address.read().to_uint64();
            if (addr < 10)
                this->ready.write(SC_LOGIC_1);
        }
        wait(this->clk.posedge_event()); // wait for sensitivity signals (address, chipSelect, read, write)
    }
}
template <int WIDTH>
void Memory<WIDTH>::checkConflict()
{
    if ((this->read.read() == SC_LOGIC_1) &&
        (this->write.read() == SC_LOGIC_1))
        SC_REPORT_FATAL("Memory", "Conflict: both read and write signals are asserted simultaneously.");
}



template <int A_WIDTH, int D_WIDTH>
class dummy_cpu : public sc_master_module<A_WIDTH, D_WIDTH> {

public:

    sc_in<sc_logic> interrupt;

    SC_HAS_PROCESS(dummy_cpu);
    dummy_cpu(sc_module_name name, sc_trace_file* tf, sc_clock& _clk)
        : sc_master_module<A_WIDTH, D_WIDTH>(name, tf, _clk) {

        SC_THREAD(program);
        this->sensitive << this->clk;
    }

    void program() {
        using addr_t = sc_lv<A_WIDTH>;
        using data_t = sc_lv<D_WIDTH>;

        this->read = SC_LOGIC_0;
        this->write = SC_LOGIC_0;
        this->address = addr_t(0);
        this->out = data_t(0);

        int i = 0;
        wait(20);
        cout << i++ << endl;

        // wordsNum = 3
        this->address = addr_t(8);
        this->out = data_t(3);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        cout << i++ << endl;

        // fromAddress = 13
        this->address = addr_t(9);
        this->out = data_t(13);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        cout << i++ << endl;
        wait(this->clk.posedge_event());

        // toAddress = 0
        this->address = addr_t(10);
        this->out = data_t(0);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        cout << i++ << endl;
        wait(this->clk.posedge_event());

        // mode = 1 (start)
        this->address = addr_t(11);
        this->out = data_t(1);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        wait(this->clk.posedge_event());
        cout << i++ << endl;

        // mode = 2 (Read from memory)
        this->address = addr_t(11);
        this->out = data_t(2);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        wait(this->clk.posedge_event());
        cout << i++ << endl;


        // wait for interrupt
        cout << "wait for interrupt" << endl;
        wait(interrupt.posedge_event());
        cout << "interrupt!" << endl;
        cout << i++ << endl;

        // Reconfigure (wordsNum = 1)
        this->address = addr_t(8);
        this->out = data_t(1);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        cout << i++ << endl;
        wait(this->clk.posedge_event());

        // fromAddress = 3
        this->address = addr_t(9);
        this->out = data_t(3);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        wait(this->clk.posedge_event());

        // toAddress = 15
        this->address = addr_t(10);
        this->out = data_t(15);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        wait(this->clk.posedge_event());

        // mode = 1 (start)
        this->address = addr_t(11);
        this->out = data_t(1);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());
        wait(this->clk.posedge_event());

        // mode = 4 (store in memory)
        this->address = addr_t(11);
        this->out = data_t(4);
        this->write = SC_LOGIC_1;
        wait(this->clk.posedge_event());
        this->write = SC_LOGIC_0;
        while (this->ready.read() != SC_LOGIC_1) wait(this->clk.posedge_event());

        // wait for interrupt
        cout << "wait for interrupt" << endl;
        wait(interrupt.posedge_event());
        cout << "interrupt!" << endl;
        cout << i++ << endl;

        sc_stop();
    }

};






int sc_main(int argc, char* argv[]) {

    sc_trace_file* tf = sc_create_vcd_trace_file("dma_test");
    cout << "hi" << endl;

    sc_clock clk{ "clk", 1, SC_NS };
    sc_trace(tf, clk, "clk");

    sc_signal<sc_logic> interrupt;
    sc_trace(tf, interrupt, "cpu/interrupt");


    dummy_cpu<Address_WIDTH, Data_WIDTH> cpu("cpu", tf, clk);
    cpu.interrupt(interrupt);

    addAccelerator<Address_WIDTH, Data_WIDTH> add("+", tf, clk, MemoryPartition::top(), 8);

    DMA<Address_WIDTH, Data_WIDTH> dma("dma", tf, clk, MemoryPartition::top(), 5);


    Memory<Address_WIDTH> memory("memory", tf, clk, MemoryPartition::top());
    dma.get_distributor_Unit()->interrupt(interrupt);

    sc_bus<2, 2, Address_WIDTH, Data_WIDTH> global_bus("globalBus", tf, clk);
    global_bus.addSlave(dma.get_config_unit());
    global_bus.addSlave(&memory);
    global_bus.addMaster(&cpu);
    global_bus.addMaster(dma.get_distributor_Unit()->memorySide);

    sc_bus<1, 1, Address_WIDTH, Data_WIDTH> dma_bus("dmaBus", tf, clk);
    dma_bus.addMaster(dma.get_distributor_Unit()->otherSide);
    dma_bus.addSlave(&add);

    sc_start();
    sc_close_vcd_trace_file(tf);

    return 0;


}