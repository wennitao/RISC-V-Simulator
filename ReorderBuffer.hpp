#ifndef RISCV_ReorderBuffer
#define RISCV_ReorderBuffer

#include "decode.hpp"

struct ROB {
    char instruction ;
    bool ready ;
    unsigned int dest, value ;
} ;

class ReorderBuffer {
public:
    ROB que[max_size] ;
    int head, tail ;

public:
    ReorderBuffer () {
        head = tail = 0 ;
    }
    
    bool full () const {
        return (tail + 1) % max_size == head ;
    }

    bool empty() const {
        return head == tail ;
    }

    int nextPos () const {
        return tail ;
    }

    void push (operation_parameter op) {
        ROB cur ;
        cur.instruction = op.TYPE; cur.dest = op.rd; cur.ready = false ;
        que[tail] = cur ;
        tail = (tail + 1) % max_size ;
    }

    std::pair<int, ROB> front () const {
        ROB cur = que[head] ;
        return make_pair (head, cur) ;
    }

    void pop () {
        head = (head + 1) % max_size ;
    }
    
    void update (int pos, unsigned int val) {
        que[pos].value = val ;
        que[pos].ready = true ;
    }
} ;

#endif