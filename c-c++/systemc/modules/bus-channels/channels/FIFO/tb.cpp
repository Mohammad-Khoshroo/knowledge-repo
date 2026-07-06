#include "FIFO_PutGet.hpp"

SC_MODULE(fifoPutGet_TB) {

    fifo<sc_lv<8>, 9>* FIFO1;
    transmitter* TRS1;
    receiver* RCV1;

    SC_CTOR(fifoPutGet_TB) {
        FIFO1 = new fifo<sc_lv<8>, 9>;
        TRS1 = new transmitter("Transmitter");
        TRS1->out(*FIFO1);
        RCV1 = new receiver("Receiver");
        RCV1->in(*FIFO1);
    }
};


SC_MODULE(fifoPutGet_TB2) {

    fifo<sc_lv<8>, 9> FIFO1;
    transmitter TRS1;
    receiver RCV1;

    SC_CTOR(fifoPutGet_TB2) :FIFO1(), TRS1("Transmitter"), RCV1("Receiver")
    {
        TRS1.out(FIFO1);
        RCV1.in(FIFO1);
    }
};

int sc_main() {
    fifoPutGet_TB FPG1("fifoPutGet1");
    sc_start();
    return 0;
}