#include <systemc.h>

template <class T>
class put_if : virtual public sc_interface {
public:
    virtual void put(int initiator, int target, T data) = 0;
};

template <class T>
class get_if : virtual public sc_interface {
public:
    virtual void get(int& initiator, int target, T& data) = 0;
};


template <class T, int numOfInitiators, int numOfTargets>
class sharedBus : public put_if<T>, public get_if<T> {

    int comingFrom, goingTo;
    T dataPlaced;

    sc_event dataAvailable[numOfTargets];
    sc_event dataReceived[numOfTargets];

    sc_mutex busBusy;

public:
    sharedBus() : comingFrom(-1), goingTo(-1) {};
    ~sharedBus() {};
    
    void put(int initiator, int target, T data) {
        busBusy.lock();
        comingFrom = initiator;
        goingTo = target;
        dataPlaced = data;
        dataAvailable[target].notify();
        wait(dataReceived[target]);
        busBusy.unlock();
    }
    
    void get(int& initiator, int target, T& data) {
        if (goingTo != target) wait(dataAvailable[target]);
        initiator = comingFrom; // set initiator in target module to the one who put the data: for logging and routing 
        data = dataPlaced;
        comingFrom = -1;
        goingTo = -1; // prevent multiple gets of same data
        dataReceived[target].notify();
    }
};