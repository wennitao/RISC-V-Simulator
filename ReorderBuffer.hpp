#ifndef RISCV_ReorderBuffer
#define RISCV_ReorderBuffer

#include "decode.hpp"

struct ROB {
    optype instruction ;
    bool ready ;
    unsigned int dest, value ;
} ;

const int max_size = 10 ;

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

    int nextPos () const {
        return tail ;
    }

    void push (operation_parameter op) {
        ROB cur ;
        cur.instruction = op.type; cur.dest = op.rd; cur.ready = false ;
        que[tail] = cur ;
        tail = (tail + 1) % max_size ;
    }

    ROB front () const {
        ROB cur = que[head] ;
        return cur ;
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