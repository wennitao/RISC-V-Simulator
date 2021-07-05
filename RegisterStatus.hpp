#ifndef RISCV_RegisterStatus
#define RISCV_RegisterStatus

struct RegisterStatus {
    bool busy ;
    unsigned int q, v ;
} ;

#endif