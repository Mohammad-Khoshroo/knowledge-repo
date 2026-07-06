#include "stack_PushPop.hpp"

SC_MODULE(stackPushPop_TB) {

    stack* STK1;
    transmitter* TRS1;
    receiver* RCV1;

    SC_CTOR(stackPushPop_TB) {
        STK1 = new stack();
        TRS1 = new transmitter("Transmitter");
        TRS1->out(*STK1);
        RCV1 = new receiver("Receiver");
        RCV1->in(*STK1);
    }
};

int main() {
    stackPushPop_TB SPP1("stackPushPop1");
    sc_start();
    return 0;
}