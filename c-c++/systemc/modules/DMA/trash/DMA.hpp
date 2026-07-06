#ifndef DMA_HPP
#define DMA_HPP

#include "slave_module.hpp"
#include "master_module.hpp"

template<int A_WIDTH, int D_WIDTH>
class DMA_DistributorUnit;

// ===========================
// DMA_ConfigUnit (slave)
// ===========================

template<int A_WIDTH, int D_WIDTH>
class DMA_ConfigUnit : public sc_slave_module<A_WIDTH, D_WIDTH> {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

    void evalConfig();

    sc_signal<data_t> is_done;
    sc_signal<addr_t> fromAddress;
    sc_signal<addr_t> toAddress;
    sc_signal<data_t> wordsNum;
    sc_signal<data_t> mode; // start/ RD/ WR

    template<int, int> friend class DMA_DistributorUnit;

public:

    SC_HAS_PROCESS(DMA_ConfigUnit);
    DMA_ConfigUnit(sc_module_name name, sc_trace_file* tf, sc_clock& _clk, int startAddress, int memorySize)
        : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, _clk, startAddress, memorySize) {

        sc_trace(tf, is_done, (std::string(name) + "/is_done").c_str());
        sc_trace(tf, fromAddress, (std::string(name) + "/fromAddress").c_str());
        sc_trace(tf, toAddress, (std::string(name) + "/toAddress").c_str());
        sc_trace(tf, wordsNum, (std::string(name) + "/wordsNum").c_str());
        sc_trace(tf, mode, (std::string(name) + "/mode").c_str());

        SC_THREAD(evalConfig);
        this->sensitive << this->clk.pos();
    }
};

template<int A_WIDTH, int D_WIDTH>
void DMA_ConfigUnit<A_WIDTH, D_WIDTH>::evalConfig() {
    while (true) {

        this->ready = SC_LOGIC_1;
        this->out = 0;

        do { wait(this->clk.posedge_event()); } while (this->chipSelect != SC_LOGIC_1);

        this->ready = SC_LOGIC_0;

        uint64_t localAddress = this->address.read().to_uint64();

        if (this->write == SC_LOGIC_1) {
            switch (localAddress) {
            case 0:cout << this->input.read() << "wordsNum" << endl; wordsNum = this->input; break;
            case 1:cout << this->input.read() << "fromAddress" << endl; fromAddress = this->input; break;
            case 2:cout << this->input.read() << "toAddress" << endl; toAddress = this->input; break;
            case 3:cout << this->input.read() << "mode" << endl; mode = this->input; break;
            default:
                SC_REPORT_ERROR("DMA", "Invalid address in config state.");
                break;
            }
        }
        else if (this->read == SC_LOGIC_1) {
            if (localAddress == 4)
                this->out = is_done;
            else
                SC_REPORT_ERROR("DMA", "Invalid address for process check.");
        }

        this->ready = SC_LOGIC_1;

        wait(this->clk.posedge_event());
    }
}

// ===========================
// Distributor sub-modules
// ===========================

template<int A_WIDTH, int D_WIDTH>
class DMA_Distributor_OtherSide : public sc_master_module<A_WIDTH, D_WIDTH> {
public:
    SC_HAS_PROCESS(DMA_Distributor_OtherSide);
    DMA_Distributor_OtherSide(sc_module_name name, sc_trace_file* tf, sc_clock& _clk)
        : sc_master_module<A_WIDTH, D_WIDTH>(name, tf, _clk) {}
};

template<int A_WIDTH, int D_WIDTH>
class DMA_Distributor_MemorySide : public sc_master_module<A_WIDTH, D_WIDTH> {
public:
    SC_HAS_PROCESS(DMA_Distributor_MemorySide);
    DMA_Distributor_MemorySide(sc_module_name name, sc_trace_file* tf, sc_clock& _clk)
        : sc_master_module<A_WIDTH, D_WIDTH>(name, tf, _clk) {}
};

// ===========================
// DMA_DistributorUnit
// ===========================

template<int A_WIDTH, int D_WIDTH>
class DMA_DistributorUnit : public sc_module {

private:

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

    void startTransfer();
    void fromMemoryToConsumer(int offset);
    void fromProducerToMemory(int offset);
    void eval();

    sc_signal<data_t> tempReg;
    DMA_ConfigUnit<A_WIDTH, D_WIDTH>* configUnit;

public:

    sc_in<bool> clk;
    sc_out<sc_logic> interrupt;

    DMA_Distributor_MemorySide<A_WIDTH, D_WIDTH>* memorySide;
    DMA_Distributor_OtherSide<A_WIDTH, D_WIDTH>* otherSide;

    SC_HAS_PROCESS(DMA_DistributorUnit);
    DMA_DistributorUnit(sc_module_name name, sc_trace_file* tf, sc_clock& _clk, DMA_ConfigUnit<A_WIDTH, D_WIDTH>* configUnit)
        : sc_module(name),
        clk(_clk),
        configUnit(configUnit)
    {
        memorySide = new DMA_Distributor_MemorySide<A_WIDTH, D_WIDTH>((std::string(name) + "_memory_side").c_str(), tf, _clk);
        otherSide = new DMA_Distributor_OtherSide<A_WIDTH, D_WIDTH>((std::string(name) + "_other_side").c_str(), tf, _clk);

        SC_THREAD(eval);
        sensitive << clk;
    }
};

template<int A_WIDTH, int D_WIDTH>
void DMA_DistributorUnit<A_WIDTH, D_WIDTH>::startTransfer() {

    while (configUnit->mode.read()[0] != '1') wait(clk.posedge_event());

    configUnit->is_done = 0;
    data_t m = configUnit->mode.read();
    m[0] = '0';
    configUnit->mode.write(m);


    memorySide->address = 0;
    memorySide->out = 0;
    memorySide->read = SC_LOGIC_0;
    memorySide->write = SC_LOGIC_0;

    otherSide->address = 0;
    otherSide->out = 0;
    otherSide->write = SC_LOGIC_0;
    otherSide->read = SC_LOGIC_0;

}

template<int A_WIDTH, int D_WIDTH>
void DMA_DistributorUnit<A_WIDTH, D_WIDTH>::fromMemoryToConsumer(int offset) {

    memorySide->address = (configUnit->fromAddress).read().to_uint64() + offset;
    memorySide->write = SC_LOGIC_0;
    memorySide->read = SC_LOGIC_1;
    do { wait(clk.posedge_event()); } while (memorySide->ready != SC_LOGIC_1);
    memorySide->read = SC_LOGIC_0;

    tempReg = memorySide->input;

    otherSide->address = (configUnit->toAddress).read().to_uint64() + offset;
    otherSide->read = SC_LOGIC_0;
    otherSide->write = SC_LOGIC_1;
    otherSide->out = tempReg;
    do { wait(clk.posedge_event()); } while (otherSide->ready != SC_LOGIC_1);
    otherSide->write = SC_LOGIC_0;
}

template<int A_WIDTH, int D_WIDTH>
void DMA_DistributorUnit<A_WIDTH, D_WIDTH>::fromProducerToMemory(int offset) {
    otherSide->address = (configUnit->fromAddress).read().to_uint64() + offset;
    otherSide->read = SC_LOGIC_1;
    otherSide->write = SC_LOGIC_0;
    do { wait(clk.posedge_event()); } while (otherSide->ready != SC_LOGIC_1);
    otherSide->read = SC_LOGIC_0;

    tempReg = otherSide->input;

    memorySide->address = (configUnit->toAddress).read().to_uint64() + offset;
    memorySide->write = SC_LOGIC_1;
    memorySide->read = SC_LOGIC_0;
    memorySide->out = tempReg;
    do { wait(clk.posedge_event()); } while (memorySide->ready != SC_LOGIC_1);
    memorySide->write = SC_LOGIC_0;

}

template<int A_WIDTH, int D_WIDTH>
void DMA_DistributorUnit<A_WIDTH, D_WIDTH>::eval() {
    while (true) {
        startTransfer();

        while ((configUnit->mode.read()[1] != '1') && (configUnit->mode.read()[2] != '1')) wait(clk.posedge_event());

        uint16_t wordsNum = (configUnit->wordsNum).read().to_uint64();
        for (int offset = 0; offset < wordsNum; ++offset) {
            if (configUnit->mode.read()[1] == '1' && configUnit->mode.read()[2] == '1')
                SC_REPORT_ERROR("DMA", "Invalid config: both WR and RD are set.");

            if (configUnit->mode.read()[1] == '1')
                fromMemoryToConsumer(offset);
            else if (configUnit->mode.read()[2] == '1')
                fromProducerToMemory(offset);

        }

        interrupt = SC_LOGIC_1;
        configUnit->is_done = 1;
        wait();
        interrupt = SC_LOGIC_0;
        configUnit->is_done = 0;
    }
}

// ===========================
// DMA Wrapper Module
// ===========================

template<int A_WIDTH, int D_WIDTH>
class DMA : public sc_module {

    DMA_ConfigUnit<A_WIDTH, D_WIDTH>* configUnit;
    DMA_DistributorUnit<A_WIDTH, D_WIDTH>* distributorUnit;

public:

    DMA_ConfigUnit<A_WIDTH, D_WIDTH>* get_config_unit() { return configUnit; }
    DMA_DistributorUnit<A_WIDTH, D_WIDTH>* get_distributor_Unit() { return distributorUnit; }

    SC_HAS_PROCESS(DMA);
    DMA(sc_module_name name, sc_trace_file* tf, sc_clock& _clk, int startMemory, int memorySize)
        : sc_module(name)
    {
        configUnit = new DMA_ConfigUnit<A_WIDTH, D_WIDTH>((std::string(name) + "_config").c_str(), tf, _clk, startMemory, memorySize);
        distributorUnit = new DMA_DistributorUnit<A_WIDTH, D_WIDTH>((std::string(name) + "_dist").c_str(), tf, _clk, get_config_unit());
    }

    ~DMA() {
        delete configUnit;
        delete distributorUnit;
    }
};

#endif // DMA_HPP
