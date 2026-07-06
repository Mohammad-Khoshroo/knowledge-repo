#include <systemc.h>

class put_if : virtual public sc_interface {
public:
    virtual void put(sc_lv<8>) = 0;
};

class get_if : virtual public sc_interface {
public:
    virtual void get(sc_lv<8>&) = 0;
};

class buffer : public put_if, public get_if {
    bool full;
    sc_lv<8> saved;
    sc_event put_event, get_event;
public:
    buffer() : full(false) {};
    ~buffer() {};
    void put(sc_lv<8> data);
    void get(sc_lv<8>& data);
};

void buffer::put(sc_lv<8> data) {

    if (full == true) wait(get_event);
    saved = data;
    full = true;
    put_event.notify();
}

void buffer::get(sc_lv<8>& data) {

    if (full == false) wait(put_event);
    data = saved;
    full = false;
    get_event.notify();
}