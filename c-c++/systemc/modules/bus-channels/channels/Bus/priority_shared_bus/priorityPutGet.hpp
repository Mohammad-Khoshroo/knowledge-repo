#include "priority_sharedBus_channel.hpp"

// initiator Number, Frequency, Delay
template <int N, int F, int D>
SC_MODULE(initiators) {
    sc_port<put_if<sc_lv<8>>> out;

    SC_CTOR(initiators) {
        SC_THREAD(putting);
    }
    void putting();

};

template <int N, int F, int D>
void initiators<N, F, D>::putting() {
    int toTarget;
    sc_lv<8> tData; // transmitted Data

    for (int i = (N * 16); i < (N * 16 + 15); i++) {
        
        wait(F, SC_NS);
        tData = (sc_lv<8>) (rand() % 256);
        toTarget = 0;

        out->put(N, toTarget, tData, D);
    }
}

template <int N, int F, int D>
SC_MODULE(targets) {
    sc_port<get_if<sc_lv<8>>> in;

    SC_CTOR(targets) {
        SC_THREAD(getting);
    }
    void getting();
};

template <int N, int F, int D>
void targets<N, F, D>::getting() {
    sc_lv<8> rData; // received Data
    int dataInitiator;

    while (1)
        wait(F, SC_NS);
    in->get(dataInitiator, N, rData, D);

}