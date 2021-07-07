#ifndef RISCV_RegisterStatus
#define RISCV_RegisterStatus

struct RegisterStatus {
    bool busy ;
    unsigned int v ;
    int q = -1 ;
} ;

#endif