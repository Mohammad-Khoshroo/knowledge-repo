#include "sharedBus_channel.hpp"

template <int N, int F>
SC_MODULE(initiators) {

    sc_port<put_if<sc_lv<8>>> out;

    SC_CTOR(initiators) {
        SC_THREAD(putting);
    }
    void putting();
};

template <int N, int F>
void initiators<N, F>::putting() {
    int toTarget;
    sc_lv<8> transmittedData;
    ofstream fout("Multi-way Sharedbus report.txt", ios: : app);

    for (int i = (N * 16); i < (N * 16 + 15); i++)
    {
        wait(F, SC_NS);
        transmittedData = (sc_lv<8>) i;
        toTarget = rand() % 4; // Total 4 targets

        cout << "\nInitiator {" << N << "} intends to"
            << " transmit (" << transmittedData << ") at: "
            << sc_time_stamp() << " to: [" << toTarget << "]\n";

        out->put(N, toTarget, transmittedData);

        cout << "Initiator {" << N << "} completed transmitting ("
            << transmittedData << ") at: "
            << sc_time_stamp() << " to: [" << toTarget << "]\n";
    }
}


template <int N, int F>
SC_MODULE(targets) {

    sc_port<get_if<sc_lv<8>>> in;

    SC_CTOR(targets) {
        SC_THREAD(getting);
    }
    void getting();
};

template <int N, int F>
void targets<N, F>::getting() {
    
    sc_lv<8> receivedData;
    
    int dataInitiator;
    
    ofstream fout("Multi-way Sharedbus report.txt", ios: : app);

    while (1)
    {
        wait(F, SC_NS);

        cout << "Target [" << N << "] ready to" << " receive something at: "
            << sc_time_stamp() << '\n';

        in->get(dataInitiator, N, receivedData);
        
        cout << "Target [" << N << "] received (" << receivedData << ") at: "
            << sc_time_stamp() << " from: {" << dataInitiator << "}\n";

    }

}