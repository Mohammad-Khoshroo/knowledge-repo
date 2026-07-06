#include <systemc.h>

class stack_push_if : virtual public sc_interface
{
public:
    virtual bool nb_push(sc_lv<8>) = 0;
    virtual void init() = 0;
};

class stack_pop_if : virtual public sc_interface
{
public:
    virtual bool nb_pop(sc_lv<8>&) = 0;
};


class stack : public stack_push_if, public stack_pop_if {
public:
    stack() { tos = 0; };
    bool nb_push(sc_lv<8> data);
    void init();
    bool nb_pop(sc_lv<8>& data);

private:
    sc_lv<8> contents[17];
    int tos;
};

bool stack::nb_push(sc_lv<8> data) {
    if (tos < 17)
    {
        contents[tos++] = data;
        return true;
    }
    return false;
}

void stack::init() { tos = 0; }

bool stack::nb_pop(sc_lv<8>& data) {
    if (tos > 0)
    {
        data = contents[--tos];
        return true;
    }
    return false;

}