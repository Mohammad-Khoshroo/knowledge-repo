#include <systemc.h>

SC_MODULE(RTLhandshaking) {

    sc_in<sc_logic> clk, go;
    sc_out<sc_logic> ready;
    sc_in<sc_logic> serIn;
    sc_out<sc_lv<8>> stackData;
    sc_out<sc_logic> push;

    sc_signal<sc_lv<8>> collectBits;
    sc_signal<sc_logic> parAccepted, parReady;

    // Serial-to-Parallel internals
    enum S2PstateType { go1Wait, go0Wait, count8Bits, got8Bits };
    sc_signal<S2PstateType> S2Pstate;
    sc_signal<sc_lv<3>> countBits;

    // Write-to-Stack internals
    enum W2SstateType { par1Wait, par0Wait, pushData };
    sc_signal<W2SstateType> W2Sstate;
    
    SC_CTOR(RTLhandshaking) {
        SC_METHOD(S2Ptransition);
        sensitive << clk;
        SC_METHOD(S2Psignals);
        sensitive << S2Pstate;
        SC_METHOD(W2Stransition);
        sensitive << clk;
        SC_METHOD(W2Ssignals);
        sensitive << W2Sstate;
    }
    
    void S2Ptransition();
    void S2Psignals();
    void W2Stransition();
    void W2Ssignals();
};

void RTLhandshaking::S2Ptransition() {
    
    if (clk->event() && clk == '1')
        switch (S2Pstate) {
        case go1Wait:
            if (go == '1') S2Pstate = go0Wait;
            else S2Pstate = go1Wait;
            break;
        case go0Wait:
            if (go == '0') S2Pstate = count8Bits;
            else S2Pstate = go0Wait;
            countBits = 0;
            break;
        case count8Bits:
            if (countBits.read().to_uint() == 7) S2Pstate = got8Bits;
            else S2Pstate = count8Bits;
            collectBits = (serIn, collectBits.read().range(7, 1));
            countBits = countBits.read().to_uint() + 1;
            break;
        case got8Bits:
            if (parAccepted == '1') S2Pstate = go1Wait;
            else S2Pstate = got8Bits;
            break;
        default:
            break;

        }
}

void RTLhandshaking::S2Psignals() {
    ready = SC_LOGIC_0;
    parReady = SC_LOGIC_0;
    if (S2Pstate == go1Wait) ready = SC_LOGIC_1;
    else if (S2Pstate == got8Bits) parReady = SC_LOGIC_1;
}

void RTLhandshaking::W2Stransition() {
    if (clk->event() && clk == '1')
        switch (W2Sstate) {
        case par1Wait:
            if (parReady == '1') W2Sstate = par0Wait;
            else W2Sstate = par1Wait;
            break;
        case par0Wait:
            if (parReady == '0') W2Sstate = pushData;
            else W2Sstate = par0Wait;
            break;
        case pushData:
            W2Sstate = par1Wait;
            break;
        default:
            break;
        }
}

void RTLhandshaking::W2Ssignals() {
    push = SC_LOGIC_0;
    parAccepted = SC_LOGIC_0;
    stackData = 0;
    if (W2Sstate == par0Wait) {
        parAccepted = SC_LOGIC_1;
        stackData = collectBits;
    }
    else if (W2Sstate == pushData) {
        push = SC_LOGIC_1;
        stackData = collectBits;

    }
}
