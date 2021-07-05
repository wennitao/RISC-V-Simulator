#ifndef RISCV_ReservationStation
#define RISCV_ReservationStation

#include <bits/stdc++.h>

enum RS_op {
    rs_add, rs_equal, rs_less, rs_lessu
} ;

struct RS {
    bool busy ;
    unsigned int vj, qj, vk, qk, dest ;
} ;

const int max_size = 10 ;

class ReservationStation {
public:
    RS que[5][10] ;
    int head[5], tail[5] ;

public:
    ReservationStation () {
        memset (head, 0, sizeof head) ;
        memset (tail, 0, sizeof tail) ;
    }

    void push (std::pair<RS_op, RS> op) {
        int id = op.first ;
        que[id][tail[id]] = op.second ;
        tail[id] = (tail[id] + 1) % max_size ;
    }
} ;

#endif