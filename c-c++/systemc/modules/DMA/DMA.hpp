#ifndef DMA_HPP
#define DMA_HPP

#include "slave_module.hpp"
#include "master_module.hpp"

using namespace sc_dt;

template<int, int>
class DMA;


template<int A_WIDTH, int D_WIDTH>
class sc_distributor_module : public sc_master_module<A_WIDTH, D_WIDTH> {

public:

    sc_distributor_module(sc_module_name name, sc_trace_file* tf) : sc_master_module<A_WIDTH, D_WIDTH>(name, tf) {};

};




template<int A_WIDTH, int D_WIDTH>
class DMA_ConfigUnit : public sc_slave_module<A_WIDTH, D_WIDTH> {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

protected:

    sc_signal<sc_logic> is_done;

    sc_signal<data_t> _wordsNum;
    sc_signal<addr_t> _fromAddress;  //based on global address mapping
    sc_signal<addr_t> _toAddress;    //based on exclusive DMA bus address mapping
    sc_signal<data_t> _mode;         // config_is_done/ RD/ WR

    template<int, int> friend class sc_distributor_module;
    template<int, int> friend class DMA;


public:

    DMA_ConfigUnit(sc_module_name name, sc_trace_file* tf, sc_address_space& space, int memorySize)
        : sc_slave_module<A_WIDTH, D_WIDTH>(name, tf, space, memorySize) {

        std::string prefix = std::string(name);
        sc_trace(tf, is_done, (prefix + "/is_done").c_str());
        sc_trace(tf, _fromAddress, (prefix + "/_fromAddress").c_str());
        sc_trace(tf, _toAddress, (prefix + "/_toAddress").c_str());
        sc_trace(tf, _wordsNum, (prefix + "/_wordsNum").c_str());
        sc_trace(tf, _mode, (prefix + "/_mode").c_str());

    }
};

template<int A_WIDTH, int D_WIDTH>
class DMA : public sc_module {

    using addr_t = sc_lv<A_WIDTH>;
    using data_t = sc_lv<D_WIDTH>;

private:


    void initialize() {

        config->is_done = SC_LOGIC_0; //false

        global->address = 0;
        global->out = 0;
        global->read = SC_LOGIC_0;
        global->write = SC_LOGIC_0;

        exclusive->address = 0;
        exclusive->out = 0;
        exclusive->write = SC_LOGIC_0;
        exclusive->read = SC_LOGIC_0;

    }

    void transfer_from_memory_to_other(int offset) {

        global->address = config->_fromAddress.read().to_uint64() + offset;

        global->write = SC_LOGIC_0;
        global->read = SC_LOGIC_1;

        wait(clk.posedge_event());
        do { wait(clk.posedge_event()); } while (global->ready.read() != SC_LOGIC_1);

        global->write = SC_LOGIC_0;
        global->read = SC_LOGIC_0;

        // cout << "addr set to : " << config->_toAddress.read().to_uint64() + offset << endl;

        exclusive->address = config->_toAddress.read().to_uint64() + offset;


        cout << global->input.read() << endl;

        exclusive->out = global->input;

        exclusive->write = SC_LOGIC_1;
        exclusive->read = SC_LOGIC_0;

        wait(clk.posedge_event());
        do { wait(clk.posedge_event()); } while (exclusive->ready.read() != SC_LOGIC_1);

        exclusive->write = SC_LOGIC_0;
        exclusive->read = SC_LOGIC_0;

    }

    void transfer_from_other_to_memory(int offset) {

        exclusive->address = config->_fromAddress.read().to_uint64() + offset;

        exclusive->write = SC_LOGIC_0;
        exclusive->read = SC_LOGIC_1;

        wait(clk.posedge_event());
        do { wait(clk.posedge_event()); } while (exclusive->ready.read() != SC_LOGIC_1);

        exclusive->write = SC_LOGIC_0;
        exclusive->read = SC_LOGIC_0;

        global->address = config->_toAddress.read().to_uint64() + offset;

        global->out = exclusive->input;

        global->write = SC_LOGIC_1;
        global->read = SC_LOGIC_0;

        wait(clk.posedge_event());
        do { wait(clk.posedge_event()); } while (global->ready.read() != SC_LOGIC_1);


        global->write = SC_LOGIC_0;
        global->read = SC_LOGIC_0;


    }

    void evalTransfer() {

        while (true) {

            // Wait until the _mode is set to "config_is_done" (_mode == 1)
            do { wait(clk.posedge_event()); } while (config->_mode.read() != 1);

            initialize();

            // Wait until the _mode is set to "RD" (_mode == 2) or "WR" (_mode == 3)
            do { wait(clk.posedge_event()); } while ((config->_mode.read() != 2) && (config->_mode.read() != 3));

            uint64_t wordsNum = config->_wordsNum.read().to_uint64();

            for (int offset = 0; offset < wordsNum; ++offset) {

                if (config->_mode.read() != 2 && config->_mode.read() != 3) {
                    SC_REPORT_FATAL("DMA", "DMA mode changed unexpectedly during transfer.");
                    break;
                }

                if (config->_mode.read() == 2)
                    transfer_from_memory_to_other(offset);

                else if (config->_mode.read() == 3)
                    transfer_from_other_to_memory(offset);

            }

            interrupt = SC_LOGIC_1;

            config->is_done = SC_LOGIC_1; // true

            wait(clk.posedge_event());

            interrupt = SC_LOGIC_0;
        }
    }

    void evalConfig() {

        while (true) {

            config->ready = SC_LOGIC_0;
            config->out = 0;

            do { wait(clk.posedge_event()); } while (config->chipSelect != SC_LOGIC_1);

            uint64_t localAddress = config->address.read().to_uint64();

            if (config->write.read() == SC_LOGIC_1) {
                switch (localAddress) {

                case 0:
                    cout << config->input.read() << ": _wordsNum" << endl;
                    config->_wordsNum = config->input.read();
                    break;

                case 1:
                    cout << config->input.read() << ": _fromAddress" << endl;
                    config->_fromAddress = config->input.read();
                    break;

                case 2:
                    cout << config->input.read() << ": _toAddress" << endl;
                    config->_toAddress = config->input.read();
                    break;

                case 3:
                    cout << config->input.read() << ": _mode" << endl;
                    config->_mode = config->input.read();
                    break;

                default:
                    SC_REPORT_ERROR("DMA", "Invalid address in config state.");
                    break;
                }
            }
            else if (config->read.read() == SC_LOGIC_1) {
                if (localAddress == 4)
                {
                    sc_lv<D_WIDTH> val = 0;
                    val[0] = config->is_done.read();
                    config->out = val;
                }
                else
                    SC_REPORT_ERROR("DMA", "Invalid address for process check.");
            }

            config->ready = SC_LOGIC_1;

            wait(clk.posedge_event());
        }
    }

public:

    sc_in<bool> clk;
    sc_out<sc_logic> interrupt;

    DMA_ConfigUnit<A_WIDTH, D_WIDTH>* config;

    sc_distributor_module<A_WIDTH, D_WIDTH>* global;
    sc_distributor_module<A_WIDTH, D_WIDTH>* exclusive;

    SC_HAS_PROCESS(DMA);
    DMA(sc_module_name name, sc_trace_file* tf, sc_address_space& space, int memorySize)
        : sc_module(name)
    {
        std::string prefix = std::string(name);
        config = new DMA_ConfigUnit<A_WIDTH, D_WIDTH>((prefix + "_config").c_str(), tf, space, memorySize);
        global = new sc_distributor_module<A_WIDTH, D_WIDTH>((prefix + "_global").c_str(), tf);
        exclusive = new sc_distributor_module<A_WIDTH, D_WIDTH>((prefix + "_exclusive").c_str(), tf);

        SC_THREAD(evalConfig);
        sensitive << clk.pos();

        SC_THREAD(evalTransfer);
        sensitive << clk.pos();
    }

};

#endif // DMA_HPP
