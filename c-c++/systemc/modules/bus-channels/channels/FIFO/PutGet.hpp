#include "FIFO_channel.hpp"

SC_MODULE(transmitter) {
    sc_port<put_if<sc_lv<8>>> out;

    SC_CTOR(transmitter) {
        SC_THREAD(putting);
    }
    void putting();
};

SC_MODULE(receiver) {
    sc_port<get_if<sc_lv<8>>> in;

    SC_CTOR(receiver) {
        SC_THREAD(getting);
    }
    void getting();
};


void transmitter::putting() {
    int i;
    sc_lv<8> dataToPut;
    for (i = 0; i < 27; i++) {
        wait(7, SC_NS);
        dataToPut = (sc_lv<8>) i;
        out->put(dataToPut);
        cout << "Data: (" << dataToPut << ") was transmitted at: " << sc_time_stamp() << '\n';
    }
}

void receiver::getting() {
    int i;
    sc_lv<8> dataThatGot;
    for (i = 0; i < 27; i++)
    {
        wait(3, SC_NS);
        in->get(dataThatGot);
        cout << "Data: (" << dataThatGot << ") was received at: "
            << sc_time_stamp() << '\n';
    }
}

