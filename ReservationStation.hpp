#ifndef RISCV_ReservationStation
#define RISCV_ReservationStation

#include <bits/stdc++.h>

#include "decode.hpp"

struct RS {
    bool busy ;
    unsigned int vj, vk, dest, imm ;
    int qj = -1, qk = -1 ;
    optype op ;

    void print() {
        printf("op:%d busy:%d vj:%u qj:%d vk:%u qk:%d dest:%u imm:%u\n", op, busy, vj, qj, vk, qk, dest, imm) ;
    }
} ;

class ReservationStation {
public:
    RS que[10] ;
    int head, tail ;

public:
    ReservationStation () {
        head = tail = 0 ;
    }

    void push (RS op) {
        que[tail] = op ;
        tail = (tail + 1) % max_size ;
    }

    bool empty () const {
        return head == tail ;
    }

    bool full () const {
        return (tail + 1) % max_size == head ;
    }

    RS front () const {
        return que[head] ;
    }

    void pop () {
        head = (head + 1) % max_size ;
    }

    void update (int rob_id, unsigned int result) {
        for (int j = head; j != tail; j = (j + 1) % max_size) {
            if (que[j].qj == rob_id) que[j].qj = -1, que[j].vj = result ;
            if (que[j].qk == rob_id) que[j].qk = -1, que[j].vk = result ;
        }
    }

    void clear() {
        head = tail = 0 ;
    }
} ;

#endif