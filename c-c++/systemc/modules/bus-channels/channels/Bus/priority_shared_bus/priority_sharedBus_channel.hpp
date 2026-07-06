#include <systemc.h>

template <class T>
class put_if : virtual public sc_interface {
public:
    virtual void put(int initiator, int target, T data, int delay) = 0;
};

template <class T>
class get_if : virtual public sc_interface {
public:
    virtual void get(int& initiator, int target, T& data, int delay) = 0;
};

// data type, number of Initiators, number of Targets
template <class T, int nI, int nT>
class prioritySBus : public put_if<T>, public get_if<T> {
private:
    int comingFrom, goingTo;
    T dataPlaced;
    sc_event dataAvailable[nT];
    sc_event dataReceived[nT];
    sc_event busReleased;

    bool requestingI[nI]; // requesting Initiators
    bool busBusy;

    int priority();

public:
    prioritySBus() : comingFrom(-1), goingTo(-1), busBusy(false) {
        for (int i = 0; i < nI; ++i)
            requestingI[i] = false;
    };
    ~prioritySBus() {};

    void put(int initiator, int target, T data, int delay);
    void get(int& initiator, int target, T& data, int delay);
};

template <class T, int nI, int nT>
int prioritySBus<T, nI, nT>::priority() {
 
    // 0 has highest priority
    for (int i = 0; i < nI; ++i)
        if (requestingI[i])
            return i;

    return -1;

};

template <class T, int nI, int nT>
void prioritySBus<T, nI, nT>::put(int initiator, int target, T data, int delay) {
    
    wait(delay, SC_NS);
    requestingI[initiator] = true;
    while (busBusy || (initiator != priority()))
    {
        wait(busReleased);

        requestingI[initiator] = false;
        busBusy = true;
        comingFrom = initiator;
        goingTo = target;
        dataPlaced = data;
        dataAvailable[target].notify();
        wait(dataReceived[target]);
        busBusy = false;
        busReleased.notify();
    }
};

template <class T, int nI, int nT>
void prioritySBus<T, nI, nT>::get(int& initiator, int target, T& data, int delay) {
    if (goingTo != target)
        wait(dataAvailable[target]);
    initiator = comingFrom;
    data = dataPlaced;
    wait(delay, SC_NS);
    comingFrom = -1;
    goingTo = -1; // prevent multiple gets of same data
    dataReceived[target].notify();
};