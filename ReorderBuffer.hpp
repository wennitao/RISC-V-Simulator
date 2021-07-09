#ifndef RISCV_ReorderBuffer
#define RISCV_ReorderBuffer

#include "decode.hpp"

struct ROB {
    char instruction ;
    optype op ;
    bool ready = 0 ;
    unsigned int dest, value, pc ;
    int LSBuffer_id ;

    void print() const {
        printf("instruction:%c op:%d ready:%d dest:%u value:%u pc:%x\n", instruction, op, ready, dest, value, pc) ;
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

    void print() const {
        printf ("---------------------------\nprint ROB\n") ;
        for (int i = head; i != tail; i = (i + 1) % max_size)
            printf("i:%d ", i), que[i].print() ;
        printf("---------------------------\n") ;
    }

    int nextPos () const {
        return tail ;
    }

    void push (ROB cur) {
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