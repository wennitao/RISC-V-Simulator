#ifndef RISCV_decode
#define RISCV_decode

#include <bits/stdc++.h>

using namespace std;

enum optype {
    lui, auipc, jal, jalr, 
    beq, bne, blt, bge, bltu, bgeu, 
    lb, lh, lw, lbu, lhu, 
    sb, sh, sw, 
    addi, slti, sltiu, xori, ori, andi, slli, srli, srai, 
    add, sub, sll, slt, sltu, _xor, srl, sra, _or, _and, 
    ret
} ;

struct operation_parameter {
    char TYPE ;
    optype type ;
    unsigned int rs, rt, rd, imm, shamt, pc ;
} ;
unsigned int sext (unsigned int x, int bit) {
    if ((x >> bit) & 1) {
        unsigned int tmp = ~((1u << (bit + 1)) - 1) ;
        x = x | tmp ;
    }
    return x ;
}

operation_parameter decode_op (unsigned int op) {
    operation_parameter result ;
    if (op == 0x0ff00513) {
        result.type = ret ;
        return result ;
    }

    unsigned int opcode = op & 127 ;
    if (opcode == 55) {
        result.TYPE = 'U' ;
        result.type = lui ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.imm = (op >> 12) << 12 ;
    } else if (opcode == 23) {
        result.TYPE = 'U' ;
        result.type = auipc ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.imm = (op >> 12) << 12 ;
    } else if (opcode == 111) {
        result.TYPE = 'J' ;
        result.type = jal ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.imm = ((op >> 12) & ((1 << 8) - 1)) << 12 ;
        result.imm |= ((op >> 20) & 1) << 11 ;
        result.imm |= ((op >> 21) & ((1 << 10) - 1)) << 1 ;
        result.imm |= (op >> 31) << 20 ;
        result.imm = sext (result.imm, 20) ;
    } else if (opcode == 103) {
        result.TYPE = 'I' ;
        result.type = jalr ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.rs = (op >> 15) & ((1 << 5) - 1) ;
        result.imm = sext (op >> 20, 11) ;
    } else if (opcode == 99) {
        result.TYPE = 'B' ;
        result.rs = (op >> 15) & ((1 << 5) - 1) ;
        result.rt = (op >> 20) & ((1 << 5) - 1) ;
        result.imm = ((op >> 7) & 1) << 11 ;
        result.imm |= ((op >> 8) & ((1 << 4) - 1)) << 1 ;
        result.imm |= ((op >> 25) & ((1 << 6) - 1)) << 5 ;
        result.imm |= (op >> 31) << 12 ;
        result.imm = sext (result.imm, 12) ;

        unsigned int funct3 = (op >> 12) & 7 ;
        if (funct3 == 0) {
            result.type = beq ;
        } else if (funct3 == 1) {
            result.type = bne ;
        } else if (funct3 == 4) {
            result.type = blt ;
        } else if (funct3 == 5) {
            result.type = bge ;
        } else if (funct3 == 6) {
            result.type = bltu ;
        } else if (funct3 == 7) {
            result.type = bgeu ;
        }
    } else if (opcode == 3) {
        result.TYPE = 'L' ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.rs = (op >> 15) & ((1 << 5) - 1) ;
        result.imm = sext (op >> 20, 11) ;

        unsigned int funct3 = (op >> 12) & 7 ;
        if (funct3 == 0) {
            result.type = lb ;
        } else if (funct3 == 1) {
            result.type = lh ;
        } else if (funct3 == 2) {
            result.type = lw ;
        } else if (funct3 == 4) {
            result.type = lbu ;
        } else if (funct3 == 5) {
            result.type = lhu ;
        }
    } else if (opcode == 35) {
        result.TYPE = 'S' ;
        result.rs = (op >> 15) & ((1 << 5) - 1) ;
        result.rt = (op >> 20) & ((1 << 5) - 1) ;
        result.imm = (op >> 7) & ((1 << 5) - 1) ;
        result.imm |= (op >> 25) << 5 ;
        result.imm = sext (result.imm, 11) ;

        unsigned int funct3 = (op >> 12) & 7 ;
        if (funct3 == 0) {
            result.type = sb ;
        } else if (funct3 == 1) {
            result.type = sh ;
        } else if (funct3 == 2) {
            result.type = sw ;
        }
    } else if (opcode == 19) {
        result.TYPE = 'I' ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.rs = (op >> 15) & ((1 << 5) - 1) ;
        result.imm = sext (op >> 20, 11) ;

        unsigned int funct3 = (op >> 12) & 7 ;
        unsigned int funct7 = op >> 25 ;
        if (funct3 == 0) {
            result.type = addi ;
        } else if (funct3 == 1) {
            if (funct7 == 0) {
                result.type = slli ;
                result.shamt = (op >> 20) & ((1 << 5) - 1) ;
                result.imm = 0 ;
            }
        } else if (funct3 == 2) {
            result.type = slti ;
        } else if (funct3 == 3) {
            result.type = sltiu ;
        } else if (funct3 == 4) {
            result.type = xori ;
        } else if (funct3 == 5) {
            if (funct7 == 0) {
                result.type = srli ;
                result.shamt = (op >> 20) & ((1 << 5) - 1) ;
                result.imm = 0 ;
            } else if (funct7 == 32) {
                result.type = srai ;
                result.shamt = (op >> 20) & ((1 << 5) - 1) ;
                result.imm = 0 ;
            }
        } else if (funct3 == 6) {
            result.type = ori ;
        } else if (funct3 == 7) {
            result.type = andi ;
        }
        
    } else if (opcode == 51) {
        result.TYPE = 'R' ;
        result.rd = (op >> 7) & ((1 << 5) - 1) ;
        result.rs = (op >> 15) & ((1 << 5) - 1) ;
        result.rt = (op >> 20) & ((1 << 5) - 1) ;

        unsigned int funct3 = (op >> 12) & 7 ;
        unsigned int funct7 = op >> 25 ;
        if (funct3 == 0) {
            if (funct7 == 0) {
                result.type = add ;
            } else if (funct7 == 32) {
                result.type = sub ;
            }
        } else if (funct3 == 1) {
            result.type = sll ;
        } else if (funct3 == 2) {
            result.type = slt ;
        } else if (funct3 == 3) {
            result.type = sltu ;
        } else if (funct3 == 4) {
            result.type = _xor ;
        } else if (funct3 == 5) {
            if (funct7 == 0) {
                result.type = srl ;
            } else if (funct7 == 32) {
                result.type = sra ;
            }
        } else if (funct3 == 6) {
            result.type = _or ;
        } else if (funct3 == 7) {
            result.type = _and ;
        }
    }
    return result ;
}
#endif