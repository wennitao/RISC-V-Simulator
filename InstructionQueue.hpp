#ifndef RISCV_InstructionQueue
#define RISCV_InstructionQueue

#include "decode.hpp"

const int max_size = 10 ;

class InstructionQueue {
public:
    operation_parameter que[max_size] ;
    int head, tail ;

public:
    InstructionQueue() {
        head = tail = 0 ;
    }

    bool full () const {
        return (tail + 1) % max_size == head ;
    }

    void push (operation_parameter op) {
        que[tail] = op ;
        tail = (tail + 1) % max_size ;
    }

    operation_parameter front () const {
        operation_parameter result = que[head] ;
        return result ;
    }

    void pop () {
        head = (head + 1) % max_size ;
    }

    void clear() {
        head = tail = 0 ;
    }
} ;

#endif