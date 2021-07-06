#ifndef RISCV_ReservationStation
#define RISCV_ReservationStation

#include <bits/stdc++.h>

enum RS_op {
    rs_add, rs_sub, rs_equal, rs_less, rs_lessu, rs_xor, rs_or, rs_and, rs_sll, rs_srl
} ;

RS_op RS_ops[10] = {rs_add, rs_sub, rs_equal, rs_less, rs_lessu, rs_xor, rs_or, rs_and, rs_sll, rs_srl} ;

struct RS {
    bool busy ;
    unsigned int vj, qj = -1, vk, qk = -1, dest ;
} ;

class ReservationStation {
public:
    RS que[10][10] ;
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

    bool empty (int op) const {
        return head[op] == tail[op] ;
    }

    RS front (int op) const {
        RS res = que[op][head[op]] ;
        return res ;
    }

    void pop (int op) {
        head[op] = (head[op] + 1) % max_size ;
    }

    void update (int rob_id, unsigned int result) {
        for (int i = 0; i < 10; i ++)
            for (int j = head[i]; j != tail[i]; j = (j + 1) % max_size) {
                if (que[i][j].qj == rob_id) que[i][j].qj = -1, que[i][j].vj = result ;
                if (que[i][j].qk == rob_id) que[i][j].qk = -1, que[i][j].vk = result ;
            }
    }
} ;

#endif