#include <systemc.h>

template<class T>
class put_if : virtual public sc_interface {
public:
    virtual void put(T) = 0;
};

template<class T>
class get_if : virtual public sc_interface {
public:
    virtual void get(T&) = 0;
};

template <class T, int Max>
class fifo : public put_if<T>, public get_if<T> {

    int size;
    int elems, head;
    T queueContents[Max];
    sc_event put_event, get_event;

public:

    fifo() : size(Max), elems(0), head(0) {};
    ~fifo() {};
    
    void put(T data) {
        if (elems == size) wait(get_event);
        queueContents[(head + elems) % size] = data;
        elems = elems + 1;
        put_event.notify();
    }
    
    void get(T& data) {
        if (elems == 0) wait(put_event);
        data = queueContents[head];
        elems = elems - 1;
        head = (head + 1) % size;
        get_event.notify();
    }
};