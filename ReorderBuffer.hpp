#ifndef RISCV_ReorderBuffer
#define RISCV_ReorderBuffer

#include "decode.hpp"

struct ROB {
    char instruction ;
    optype op ;
    bool ready ;
    unsigned int dest, value, pc ;

    void print() {
        printf("instruction:%c op:%d ready:%d dest:%u value:%u pc:%u\n", instruction, op, ready, dest, value, pc) ;
    }
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
        cur.op = op.type; cur.pc = op.pc ;
        que[tail] = cur ;
        tail = (tail + 1) % max_size ;
    }

    ROB front () const {
        return que[head] ;
    }

    void pop () {
        head = (head + 1) % max_size ;
    }
    
    void update (int pos, unsigned int val) {
        que[pos].value = val ;
        que[pos].ready = true ;
    }

    void clear() {
        head = tail = 0 ;
    }
} ;

#endif