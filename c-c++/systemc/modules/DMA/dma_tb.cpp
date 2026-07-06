#include "DMA.hpp"
#include "bus.hpp"

#define Address_WIDTH 16
#define Data_WIDTH 16

template <int A_WIDTH, int D_WIDTH>
class accelerator : public sc_slave_module<A_WIDTH, D_WIDTH> {

    using data_t = sc_lv<D_WIDTH>;
    using addr_t = sc_lv<A_WIDTH>;

private:
    sc_signal<data_t> localReg[4];

public:

    SC_HAS_PROCESS(accelerator);
    accelerator(sc_module_name name, sc_trace_file* tf, sc_address_space& space, int size)
        : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, space, size) {

        localReg[0] = 0;
        localReg[1] = 0;
        localReg[2] = 0;
        localReg[3] = 0;

        std::string prefix = std::string(name);

        sc_trace(tf, localReg[0], (prefix + "/localReg(0)").c_str());
        sc_trace(tf, localReg[1], (prefix + "/localReg(1)").c_str());
        sc_trace(tf, localReg[2], (prefix + "/localReg(2)").c_str());
        sc_trace(tf, localReg[3], (prefix + "/localReg(3)").c_str());

        SC_THREAD(respond);
        this->sensitive << this->clk.pos();
    }

    void respond() {

        while (true) {

            this->ready = SC_LOGIC_0;
            this->out = 0;

            do { wait(this->clk.posedge_event()); } while (this->chipSelect != SC_LOGIC_1);

            uint64_t addr = this->address.read().to_uint64();

            if (addr > 3)
                SC_REPORT_FATAL("Accelerator", "Access out of bounds");

            else if (this->write.read() == SC_LOGIC_1) {

                localReg[addr] = this->input.read().to_int64();

            }

            else if (this->read.read() == SC_LOGIC_1) {

                if (addr == 3) {
                    sc_int<64> sum = localReg[0].read().to_int64() + localReg[1].read().to_int64() + localReg[2].read().to_int64();
                    this->out = sum;
                    localReg[3] = sum;
                }
                else
                    this->out = localReg[addr];

            }
            this->ready = SC_LOGIC_1;

            wait(this->clk.posedge_event());

            cout << "update:" << endl;
            cout << "reg0 =" << localReg[0].read() << endl;
            cout << "reg1 =" << localReg[1].read() << endl;
            cout << "reg2 =" << localReg[2].read() << endl;
            cout << "reg3 =" << localReg[3].read() << endl;

        }

    }


};


template <int WIDTH>
class Memory : public sc_slave_module<WIDTH, WIDTH>
{

private:
    int memory[10]{ 3,4,7,1,23,66,3,2,4,10 };

    void eval();
public:
    SC_HAS_PROCESS(Memory);
    Memory(sc_module_name name, sc_trace_file* tf, sc_address_space& space, int memorySize = 10) :
        sc_slave_module<WIDTH, WIDTH>(name, tf, space, memorySize)
    {
        SC_THREAD(eval);
        this->sensitive << this->clk.pos();

    }
};
template <int WIDTH>
void Memory<WIDTH>::eval()
{
    bool wr = false;
    bool rd = false;
    bool cs = false;

    while (true)
    {

        this->out = 0;
        this->ready = SC_LOGIC_0;

        do { wait(this->clk.posedge_event()); } while (this->chipSelect.read() != SC_LOGIC_1);

        uint64_t addr = this->address.read().to_uint64();

        cs = (this->chipSelect.read() == SC_LOGIC_1);
        rd = (this->read.read() == SC_LOGIC_1);
        wr = (this->write.read() == SC_LOGIC_1);

        cout << cs << rd << wr << endl;

        // Conflict check
        if (rd && wr)
            SC_REPORT_FATAL("Memory", "Conflict: both read and write are asserted.");

        // Write
        if (wr)
        {
            if (addr >= 10)
                SC_REPORT_FATAL("Memory", "Write address out of bounds.");
            else {
                memory[addr] = this->input.read().to_int();
            }

        }
        // Read
        else if (rd)
        {
            // cout << "kooooochheeeee haayyyeeee shahr0000" << endl;
            if (addr >= 10)
            {
                SC_REPORT_ERROR("Memory", "Read address out of bounds.");
                this->out.write(std::string(WIDTH, 'X').c_str());
            }
            else {
                // cout << "kooooochheeeee haayyyeeee shahr" << endl;
                this->out.write(memory[addr]);
            }
        }

        this->ready = SC_LOGIC_1;

        wait(this->clk.posedge_event()); // next clock

    }
}




template <int A_WIDTH, int D_WIDTH>
class dummy_cpu : public sc_master_module<A_WIDTH, D_WIDTH> {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;
public:


    sc_in<sc_logic> interrupt;

    SC_HAS_PROCESS(dummy_cpu);
    dummy_cpu(sc_module_name name, sc_trace_file* tf)
        : sc_master_module<A_WIDTH, D_WIDTH>(name, tf) {

        SC_THREAD(program);
        this->sensitive << this->clk.pos();
    }

    void program() {



        int deadlock = 0;
        this->read = SC_LOGIC_0;
        this->write = SC_LOGIC_0;

        this->address = addr_t(0);
        this->out = data_t(0);

        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 00" << endl;
        wait(60, SC_NS);

        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 01" << endl;

        // wordsNum = 3
        this->address = addr_t(10);
        this->out = data_t(3);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 30)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);

        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 02" << endl;

        // fromAddress = 4
        this->address = addr_t((9 + 2));
        this->out = data_t(4);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        deadlock = 0;

        wait(1, SC_NS);
        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 03" << endl;

        // toAddress = 0 
        this->address = addr_t(12);
        this->out = data_t(0);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);
        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 04" << endl;

        // mode = config_is_done
        this->address = addr_t(13);
        this->out = data_t(1);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);
        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 05" << endl;

        // mode = 2 (Read from memory)
        this->address = addr_t(13);
        this->out = data_t(2);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);
        cout << "//////////////////////////////////////////////////////////" << endl;

        cout << "step 06" << endl;
        // wait for interrupt
        cout << "wait for interrupt" << endl;

        wait(interrupt.posedge_event());
        cout << "//////////////////////////////////////////////////////////" << endl;
        cout << "step 06.2" << endl;


        cout << "interrupt!" << endl;

        cout << "//////////////////////////////////////////////////////////" << endl;
        cout << "step 07" << endl;

        // wordNum =1
        this->address = addr_t(10);
        this->out = data_t(1);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);

        cout << "//////////////////////////////////////////////////////////" << endl;
        cout << "step 08" << endl;

        // fromAddress = 3 (Read from Acc)

        this->address = addr_t(11);
        this->out = data_t(3);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);
        cout << "//////////////////////////////////////////////////////////" << endl;
        cout << "step 09" << endl;

        // toAddress = 2 (Read from memory)
        this->address = addr_t(12);
        this->out = data_t(2);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);

        cout << "//////////////////////////////////////////////////////////" << endl;
        cout << "step 10" << endl;

        // mode = config_is_done 
        this->address = addr_t(13);
        this->out = data_t(1);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);

        cout << "//////////////////////////////////////////////////////////" << endl;
        cout << "step 11" << endl;
        // mode = 3 (Read from memory)
        this->address = addr_t(13);
        this->out = data_t(3);

        this->write = SC_LOGIC_1;
        this->read = SC_LOGIC_0;

        wait(this->clk.posedge_event());

        do {
            if (deadlock >= 10)
                SC_REPORT_FATAL("deadLock", "dead!");
            else
                deadlock++;
            wait(this->clk.posedge_event());
        } while (this->ready.read() != SC_LOGIC_1);

        deadlock = 0;

        this->write = SC_LOGIC_0;
        this->read = SC_LOGIC_0;

        wait(1, SC_NS);

        wait(10, SC_NS);

        sc_stop();
    }

};




template <int A_WIDTH, int D_WIDTH>
class TopSystem : public sc_module {
    // Signals
    sc_clock clk;
    sc_signal<sc_logic> interrupt;

    // Modules
    sc_address_space* globalSpace;
    sc_address_space* dmaSpace;

    sc_bus<2, 2, A_WIDTH, D_WIDTH>* global_bus;
    sc_bus<1, 1, A_WIDTH, D_WIDTH>* dma_bus;

    accelerator<A_WIDTH, D_WIDTH>* addAcc;
    Memory<A_WIDTH>* memory;
    dummy_cpu<A_WIDTH, D_WIDTH>* cpu;
    DMA<A_WIDTH, D_WIDTH>* dma;

    // Constructor

public:

    TopSystem(sc_module_name name, sc_trace_file* tf)
        :sc_module(name)
        , clk("clk", 4, SC_NS)
    {

        sc_trace(tf, clk, std::string(name) + "/clk");
        sc_trace(tf, interrupt, std::string(name) + "/interrupt");

        // Global bus (2 masters, 2 slaves)
        globalSpace = new sc_address_space;
        global_bus = new sc_bus<2, 2, A_WIDTH, D_WIDTH>("globalBus", tf);
        global_bus->clk(clk);

        // DMA bus (1 master, 1 slave)
        dmaSpace = new sc_address_space;
        dma_bus = new sc_bus<1, 1, A_WIDTH, D_WIDTH>("dmaBus", tf);
        dma_bus->clk(clk);

        // Modules
        addAcc = new accelerator<A_WIDTH, D_WIDTH>("addAcc", tf, *dmaSpace, 8);
        addAcc->clk(clk);

        memory = new Memory<A_WIDTH>("memory", tf, *globalSpace, 10);
        memory->clk(clk);

        cpu = new dummy_cpu<A_WIDTH, D_WIDTH>("cpu", tf);
        cpu->clk(clk);
        cpu->interrupt(interrupt);

        dma = new DMA<A_WIDTH, D_WIDTH>("dma", tf, *globalSpace, 10);
        dma->clk(clk);
        dma->config->clk(clk);
        dma->global->clk(clk);
        dma->exclusive->clk(clk);
        dma->interrupt(interrupt);

        // Bus connections
        global_bus->addMaster(cpu, tf);
        global_bus->addMaster(dma->global, tf);
        global_bus->addSlave(memory, tf);
        global_bus->addSlave(dma->config, tf);

        dma_bus->addMaster(dma->exclusive, tf);
        dma_bus->addSlave(addAcc, tf);
    }

};

int sc_main(int argc, char* argv[]) {
    sc_trace_file* tf = sc_create_vcd_trace_file("dma_test");

    TopSystem<Address_WIDTH, Data_WIDTH>* system = new TopSystem<Address_WIDTH, Data_WIDTH>(sc_module_name("top"), tf);

    sc_start();
    sc_close_vcd_trace_file(tf);

    return 0;
}






// int sc_main(int argc, char* argv[]) {

//     sc_trace_file* tf = sc_create_vcd_trace_file("dma_test");
//     // tf->set_time_unit(1, SC_NS);
//     // sc_set_default_time_unit(1, SC_NS);

//     cout << "hi" << endl;

//     sc_clock clk{ "clk", 1, SC_NS };
//     sc_trace(tf, clk, "clk");

//     sc_signal<sc_logic> interrupt;
//     sc_trace(tf, interrupt, "interrupt");


//     sc_address_space* globalSpace = new sc_address_space;
//     sc_bus<2, 2, Address_WIDTH, Data_WIDTH>* global_bus = new sc_bus<2, 2, Address_WIDTH, Data_WIDTH>("globalBus", tf);
//     global_bus->clk(clk);

//     sc_address_space* dmaSpace = new sc_address_space;
//     sc_bus<1, 1, Address_WIDTH, Data_WIDTH>* dma_bus = new sc_bus<1, 1, Address_WIDTH, Data_WIDTH>("dmaBus", tf);
//     dma_bus->clk(clk);

//     accelerator<Address_WIDTH, Data_WIDTH>* addAcc = new accelerator<Address_WIDTH, Data_WIDTH>("addAcc", tf, *dmaSpace, 8);
//     addAcc->clk(clk);

//     Memory<Address_WIDTH>* memory = new Memory<Address_WIDTH>("memory", tf, *globalSpace, 10);
//     memory->clk(clk);

//     dummy_cpu<Address_WIDTH, Data_WIDTH>* cpu = new  dummy_cpu<Address_WIDTH, Data_WIDTH>("cpu", tf);
//     cpu->clk(clk);
//     cpu->interrupt(interrupt);

//     DMA<Address_WIDTH, Data_WIDTH>* dma = new DMA<Address_WIDTH, Data_WIDTH>("dma", tf, *globalSpace, 10);
//     dma->clk(clk);
//     dma->config->clk(clk);
//     dma->global->clk(clk);
//     dma->exclusive->clk(clk);
//     dma->interrupt(interrupt);

//     global_bus->addMaster(dma->global, tf);
//     global_bus->addMaster(cpu, tf);
//     global_bus->addSlave(dma->config, tf);
//     global_bus->addSlave(memory, tf);

//     dma_bus->addMaster(dma->exclusive, tf);
//     dma_bus->addSlave(addAcc, tf);

//     sc_start();
//     sc_close_vcd_trace_file(tf);

//     return 0;


// }