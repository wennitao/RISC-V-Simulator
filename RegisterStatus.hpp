#ifndef RISCV_RegisterStatus
#define RISCV_RegisterStatus

struct RegisterStatus {
    bool busy = 0 ;
    unsigned int v ;
    int q = -1 ;
} ;

#endif