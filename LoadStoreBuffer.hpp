#ifndef RISCV_LoadStoreBuffer
#define RISCV_LoadStoreBuffer

struct LSBuffer {
    bool store ;
    unsigned int vj, qj = -1, vk, qk = -1, A, dest ;
} ;

const int max_size = 10 ;

class LoadStoreBuffer {
public:
    LSBuffer que[max_size] ;
    int head, tail ;

public:
    LoadStoreBuffer () {
        head = tail = 0 ;
    }

    bool full () {
        return (tail + 1) % max_size == head ;
    }

    void push (LSBuffer cur) {
        que[tail] = cur ;
        tail = (tail + 1) % max_size ;
    }

    LSBuffer front () const {
        return que[head] ;
    }

    void pop () {
        head = (head + 1) % max_size ;
    }

    void update (int rob_id, unsigned int val) {
        for (int i = head; i != tail; i = (i + 1) % max_size) {
            if (que[i].qj == rob_id) que[i].qj = -1, que[i].vj = val ;
            if (que[i].qk == rob_id) que[i].qk = -1, que[i].vk = val ;
        }
    }
} ;

#endif