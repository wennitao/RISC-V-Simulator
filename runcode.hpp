#ifndef RISCV_runcode
#define RISCV_runcode

#include <bits/stdc++.h>

#include "main.h"
#include "decode.hpp"

using namespace std;

class runcode {
private:
    operation_parameter op ;

public:
    runcode () {}
    runcode (operation_parameter _op) : op (_op) {}

    void issue (optype type, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int imm) {

    }

    void execute (optype type, unsigned int rs, unsigned int rt, unsigned int rd) {

    }

    void write_result (optype type) {
        
    }
} ;

#endif