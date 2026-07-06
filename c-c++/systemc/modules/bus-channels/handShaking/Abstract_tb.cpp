#include "Abstract.hpp"

SC_MODULE(AbstractHandshakingTB) {
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> go, serIn;
    sc_signal<sc_logic> ready;
    sc_signal<sc_lv<8>> parOut;
    sc_signal<sc_logic> push;

    AbstractHandshaking* S2W; // Ser to par and Write to stack

    SC_CTOR(AbstractHandshakingTB) {
        S2W = new AbstractHandshaking("Handshaking_TB");
        S2W->clk(clk);
        S2W->go(go);
        S2W->serIn(serIn);
        S2W->ready(ready);
        S2W->stackData(parOut);
        S2W->push(push);

        SC_THREAD(clocking);
        SC_THREAD(serialData);
    }
    void clocking();
    void serialData();
};

void AbstractHandshakingTB::clocking() {
    int i;
    clk = SC_LOGIC_0;
    for (i = 0; i <= 500; i++) {
        wait(29, SC_NS);
        clk = SC_LOGIC_1;
        wait(29, SC_NS);
        clk = SC_LOGIC_0;

    }
}

void AbstractHandshakingTB::serialData() {
    int i, j;
    go = SC_LOGIC_0;
    serIn = SC_LOGIC_0;
    for (i = 1; i < 5; i++) {
        if (ready == SC_LOGIC_1) {
            wait(31, SC_NS);
            go = SC_LOGIC_1;
            wait(73, SC_NS);
            go = SC_LOGIC_0;
            for (j = 0; j < 7; j++) {
                serIn = SC_LOGIC_0;
                wait(17 * i, SC_NS);
                serIn = SC_LOGIC_1;
                wait(23 * i, SC_NS);
                serIn = SC_LOGIC_0;
                wait(31 * i, SC_NS);
                serIn = SC_LOGIC_1;
            }
        }
        else wait(91, SC_NS);
    }
}

int sc_main(int argc, char* argv) {

    AbstractHandshakingTB* HSTB1 = new AbstractHandshakingTB("HandshakingTB");

    sc_trace_file* VCDFile;
    VCDFile = sc_create_vcd_trace_file("Handshaking");

    sc_trace(VCDFile, HSTB1->clk, "Clk");
    sc_trace(VCDFile, HSTB1->go, "GoStart");
    sc_trace(VCDFile, HSTB1->serIn, "SerialIn");
    sc_trace(VCDFile, HSTB1->ready, "Ready4Serial");
    sc_trace(VCDFile, HSTB1->parOut, "Output2Stack");
    sc_trace(VCDFile, HSTB1->push, "Push");

    sc_trace(VCDFile, HSTB1->S2W->collectBits, "collectBits");

    sc_start(4500, SC_NS);
    sc_close_vcd_trace_file(VCDFile);
    return 0;
}