#ifndef RISCV_LoadStoreBuffer
#define RISCV_LoadStoreBuffer

struct LSBuffer {
    bool store ;
    unsigned int vj, vk, A, dest ;
    int qj = -1, qk = -1 ;
    optype op ;

    void print() {
        printf("store:%d vj:%u qj:%u vk:%u qk:%u A:%u dest:%u\n", store, vj, qj, vk, qk, A, dest) ;
    }
} ;

class LoadStoreBuffer {
public:
    LSBuffer que[max_size] ;
    int head, tail ;

public:
    LoadStoreBuffer () {
        head = tail = 0 ;
    }

    bool full () const {
        return (tail + 1) % max_size == head ;
    }

    bool empty() const {
        return head == tail ;
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

    void clear() {
        head = tail = 0 ;
    }
} ;

#endif