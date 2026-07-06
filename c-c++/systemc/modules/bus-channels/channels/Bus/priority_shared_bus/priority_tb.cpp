#include "priorityPutGet.hpp"

SC_MODULE(prioritySBusPutGet_TB) {

    prioritySBus<sc_lv<8>, 4, 1>* BusA;
    initiators<0, 9, 0>* INI0;
    initiators<1, 11, 0>* INI1;
    initiators<2, 7, 0>* INI2;
    initiators<3, 15, 0>* INI3;
    targets<0, 17, 13>* TAR0;

    SC_CTOR(prioritySBusPutGet_TB) {
        BusA = new prioritySBus<sc_lv<8>, 4, 1>;

        INI0 = new initiators<0, 9, 0>("Initiator0");
        INI0->out(*BusA);
        INI1 = new initiators<1, 11, 0>("Initiator1");
        INI1->out(*BusA);
        INI2 = new initiators<2, 7, 0>("Initiator2");
        INI2->out(*BusA);
        INI3 = new initiators<3, 15, 0>("Initiator3");
        INI3->out(*BusA);

        TAR0 = new targets<0, 17, 13>("Target0");
        TAR0->in(*BusA);
    }

};

int sc_main() {
    prioritySBusPutGet_TB prioritySBus("X");
    sc_start();
    return 0;
}