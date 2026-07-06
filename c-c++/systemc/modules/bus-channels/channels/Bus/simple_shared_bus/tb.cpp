#include "PutGet.hpp"

SC_MODULE(sharedBusPutGet_TB) {

    sharedBus<sc_lv<8>, 4, 4>* BusA;
    initiators<0, 3>* INI0;
    initiators<1, 5>* INI1;
    initiators<2, 7>* INI2;
    initiators<3, 4>* INI3;
    targets<0, 7>* TAR0;
    targets<1, 4>* TAR1;
    targets<2, 6>* TAR2;
    targets<3, 5>* TAR3;

    SC_CTOR(sharedBusPutGet_TB) {
        BusA = new sharedBus<sc_lv<8>, 4, 4>;

        INI0 = new initiators<0, 3>("Initiator0");
        INI0->out(*BusA);
        INI1 = new initiators<1, 5>("Initiator1");
        INI1->out(*BusA);
        INI2 = new initiators<2, 7>("Initiator2");
        INI2->out(*BusA);
        INI3 = new initiators<3, 4>("Initiator3");
        INI3->out(*BusA);

        TAR0 = new targets<0, 7>("Target0");
        TAR0->in(*BusA);
        TAR1 = new targets<1, 4>("Target1");
        TAR1->in(*BusA);
        TAR2 = new targets<2, 6>("Target2");
        TAR2->in(*BusA);
        TAR3 = new targets<3, 5>("Target3");
        TAR3->in(*BusA);
    }
};

int sc_main() {
    sharedBusPutGet_TB multiWay("sharedBusPutGet1");
    sc_start();
    return 0;
}