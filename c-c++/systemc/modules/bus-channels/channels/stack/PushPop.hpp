#include "stack_channel.hpp"

SC_MODULE(transmitter) {
    sc_port<stack_push_if> out;

    SC_CTOR(transmitter) {
        SC_THREAD(pushSome);
    }
    void pushSome();
};

SC_MODULE(receiver) {
    sc_port<stack_pop_if> in;

    SC_CTOR(receiver) {
        SC_THREAD(popThem);
    }
    void popThem();
};


void transmitter::pushSome() {
    int i = 0;
    sc_lv<8> data;
    for (i = 0; i <= 43; i++)
    {
        data = (sc_lv<8>) i;
        wait(3, SC_NS);
        if (out->nb_push(sc_lv<8>(i)))
            cout << "Data: " << data << " was written at: "
            << sc_time_stamp() << '\n';

        else
            cout << "Data: " << data << " is lost at: "
            << sc_time_stamp() << '\n';

    }
}

void receiver::popThem() {

    int i = 0;
    sc_lv<8> data;
    for (i = 0; i <= 41; i++)
    {

        wait(7, SC_NS);
        if (in->nb_pop(data))
            cout << "Data: " << data << " popped at: "
            << sc_time_stamp() << '\n';
        else
            cout << "No data was popped at: "
            << sc_time_stamp() << '\n';
    }

}