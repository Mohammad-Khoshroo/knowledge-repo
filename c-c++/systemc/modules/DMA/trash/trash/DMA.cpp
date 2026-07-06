#include "head.hpp"
#include "slave_module.hpp"
#include "master_module.hpp"
#include "sc_cast.hpp"

template<int A_WIDTH, int D_WIDTH>
using sc_receiver_module = sc_slave_module<A_WIDTH, D_WIDTH>;
template<int A_WIDTH, int D_WIDTH>
using sc_producer_module = sc_master_module<A_WIDTH, D_WIDTH>;


template<int, int, int>
class DMA_DistributorModule;

template <int address_WIDTH, int data_WIDTH>
struct PortTypes {
    using addr_t = sc_lv<address_WIDTH>;
    using data_t = sc_lv<data_WIDTH>;
};

template<int A_WIDTH, int D_WIDTH>
class DMA_ConfigModule : public PortTypes<A_WIDTH, D_WIDTH>, public sc_module, public sc_slave_module<A_WIDTH, D_WIDTH> {

    void evalConfig();

    // port of slave module

    // sc_in<sc_logic> clk;
    // sc_in<addr_t> address;
    // sc_in<data_t> input;
    // sc_out<data_t> out;
    // sc_out<sc_logic> read;
    // sc_out<sc_logic> write;
    // sc_out<sc_logic> chipSelect;
    // sc_in<sc_logic> ready;
    // MemoryPartition memory;

    data_t is_done;   // Done
    addr_t fromAddress; // Address to copy from
    addr_t toAddress;   // Address to copy to
    data_t wordsNum;   // Number of data to copy
    data_t controlReg;  // ...|RD|WR|Start, WR : Mem -> MMA , RD : MMA -> Mem

    template<int, int, int> friend class DMA_DistributorModule;

public:


    SC_HAS_PROCESS(DMA_ConfigModule);
    DMA_ConfigModule(sc_module_name name, sc_trace_file* tf) : sc_module(name) {

        SC_THREAD(evalConfig);
        sensitive << clk;
    }
};

template<int A_WIDTH, int D_WIDTH>
void DMA_ConfigModule<A_WIDTH, D_WIDTH>::evalConfig()
{
    while (true)
    {
        ready = SC_LOGIC_1;

        // Wait for CS
        do { wait(clk->posedge_event()); } while (chipSelect != SC_LOGIC_1);

        ready = SC_LOGIC_0;
        out = 0;

        uint16_t localAddress = sc_cast::sc_lv_cast<int>(address);

        if (write == SC_LOGIC_1) {
            switch (localAddress) {
            case 0:
                controlReg = input;
                break;
            case 1:
                fromAddress = input;
                break;
            case 2:
                toAddress = input;
                break;
            case 3:
                wordsNum = input;
                break;
            default:
                break;
            }
        }
        else if (read == SC_LOGIC_1) {
            switch (localAddress) {
            case 4:
                out = is_done;
                break;
            default:
                break;
            }
        }

    }

}

template<int receiversNum, int A_WIDTH, int D_WIDTH>
class DMA_DistributorModule : public PortTypes<A_WIDTH, D_WIDTH>, public sc_module, public sc_master_module<A_WIDTH, D_WIDTH> {

    // ports of master module for global shared bus

    // sc_in<sc_logic> clk;
    // sc_out<addr_t> address;
    // sc_in<data_t> input;
    // sc_out<data_t> out;
    // sc_out<sc_logic> write;
    // sc_out<sc_logic> read;
    // sc_in<sc_logic> ready;

    DMA_ConfigModule<A_WIDTH, D_WIDTH>* configModule;

    void receiveMemoryData(int offset);
    void storeProductData();
    void startTransfer();

    void eval();


    data_t tempReg;

    // ports of master module for DMA shared Bus
    // global Address in 
    sc_out<addr_t> receiverAddress;
    sc_out<data_t> receiverInput;
    sc_in<data_t> receiverOut;

    sc_in<sc_logic> receiverReady;
    sc_out<sc_logic> receiverWrite;
    sc_out<sc_logic> receiverRead;

    SC_HAS_PROCESS(DMA_DistributorModule);
    DMA_DistributorModule(sc_module_name name, sc_trace_file* tf, DMA_ConfigModule<A_WIDTH, D_WIDTH>* configModule) :
        sc_module(name),
        configModule(configModule)
    {
        SC_THREAD(eval);
        sensitive << clk;
    }
};

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_DistributorModule<receiversNum, A_WIDTH, D_WIDTH>::startTransfer() {
    // Wait for start
    do { wait(clk->posedge_event()); } while (configModule.controlReg[0] == '0');

    configModule.is_done = 0;
    configModule.controlReg[0] = SC_LOGIC_0;
    // Set start,done to zero
    address = 0;
    out = 0;
    read = SC_LOGIC_0;
    write = SC_LOGIC_0;

    receiverAddress = 0;
    receiverInput = 0;
    receiverWrite = SC_LOGIC_0;
    receiverRead = SC_LOGIC_0;
}

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_DistributorModule<receiversNum, A_WIDTH, D_WIDTH>::receiveMemoryData(int offset) {

    // Set the memory address and control signals
    address = sc_cast::sc_lv_cast<uint64_t>(configModule.fromAddress) + offset;
    write = SC_LOGIC_0;
    read = SC_LOGIC_1;
    // Wait for the operation to finish
    do { wait(clk->posedge_event()); } while (ready != SC_LOGIC_1);
    read = SC_LOGIC_0;
    // Save the result
    tempReg = input;
    // Set the destination address and control signals
    receiverAddress = sc_cast::sc_lv_cast<uint64_t>(configModule.toAddress) + offset;
    receiverRead = SC_LOGIC_0;
    receiverWrite = SC_LOGIC_1;
    receiverInput = tempReg;

    // Write the data in one clock
    do { wait(clk->posedge_event()); } while (receiverReady != SC_LOGIC_1);

}

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_DistributorModule<receiversNum, A_WIDTH, D_WIDTH>::storeProductData() {

    // Set the source address and control signals
    receiverAddress = sc_cast::sc_lv_cast<uint64_t>(configModule.fromAddress) + offset;
    receiverRead = SC_LOGIC_1;
    receiverWrite = SC_LOGIC_0;
    // Wait for the operation to finish
    do { wait(clk->posedge_event()); } while (receiverReady != SC_LOGIC_1);

    tempReg = receiverOut;

    // Set the memory destination address and control signals and data
    address = sc_cast::sc_lv_cast<uint64_t>(configModule.toAddress) + offset;
    write = SC_LOGIC_1;
    read = SC_LOGIC_0;
    out = tempReg;

    // Wait for operation to finish
    do { wait(clk->posedge_event()); } while (ready != SC_LOGIC_1);
    write = SC_LOGIC_0;
    read = SC_LOGIC_0;

}

template<int receiversNum, int A_WIDTH, int D_WIDTH>
void DMA_DistributorModule<receiversNum, A_WIDTH, D_WIDTH>::eval() {
    while (true)
    {
        startTransfer();

        // For each byte do the transfer
        uint16_t wordsNum = sc_cast::sc_lv_cast<uint16_t>(configModule.wordsNum);
        for (int offset = 0; offset < wordsNum; offset++)
        {
            if (configModule.controlReg[1] == '1') // Write  Mem -> MMA
                receiveMemoryData(offset);

            else if (configModule.controlReg[2] == '1') // Read MMA -> Mem
                storeProductData();
        }

        // Set done to one
        configModule.is_done = 1;
    }
}


template<int receiversNum, int A_WIDTH, int D_WIDTH>
class DMA : public sc_module, public PortTypes<A_WIDTH, D_WIDTH> {

    DMA_ConfigModule<A_WIDTH, D_WIDTH> configModule;
    DMA_DistributorModule<receiversNum, A_WIDTH, D_WIDTH> distributerModule;

    sc_in<sc_logic> clk;

    SC_HAS_PROCESS(DMA);
    DMA(sc_module_name name, sc_trace_file* tf) :
        sc_module(name),
        configModule((std::string(name) + "_config").c_str(), tf),  
        distributerModule((std::string(name) + "_dist").c_str(), tf, &configModule) {};

};