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
    add, sub, sll, slt, sltu, _xor, srl, sra, _or, _and
} ;

class decode {
private:
    unsigned int op ;

public:
    decode () {}
    decode (unsigned int _op) : op (_op) {}

    optype decode_op () {
        unsigned int opcode = op & 127 ;
        if (opcode == 55) {
            cout << "lui" << endl ;
            return lui ;
        } else if (opcode == 23) {
            cout << "auipc" << endl ;
            return auipc ;
        } else if (opcode == 111) {
            cout << "jal" << endl ;
            return jal ;
        } else if (opcode == 103) {
            cout << "jalr" << endl ;
            return jalr ;
        } else if (opcode == 99) {
            cout << "B Type" << endl ;
            unsigned int funct3 = op & (7 << 12) ;
            if (funct3 == 0) {
                cout << "beq" << endl ;
                return beq ;
            } else if (funct3 == 1) {
                cout << "bne" << endl ;
                return bne ;
            } else if (funct3 == 4) {
                cout << "blt" << endl ;
                return blt ;
            } else if (funct3 == 5) {
                cout << "bge" << endl ;
                return bge ;
            } else if (funct3 == 6) {
                cout << "bltu" << endl ;
                return bltu ;
            } else if (funct3 == 7) {
                cout << "bgeu" << endl ;
                return bgeu ;
            }
        } else if (opcode == 3) {
            cout << "I Type" << endl ;
            unsigned int funct3 = (op & (7 << 12)) >> 12 ;
            if (funct3 == 0) {
                cout << "lb" << endl ;
                return lb ;
            } else if (funct3 == 1) {
                cout << "lh" << endl ;
                return lh ;
            } else if (funct3 == 2) {
                cout << "lw" << endl ;
                return lw ;
            } else if (funct3 == 4) {
                cout << "lbu" << endl ;
                return lbu ;
            } else if (funct3 == 5) {
                cout << "lhu" << endl ;
                return lhu ;
            }
        } else if (opcode == 35) {
            cout << "S Type" << endl ;
            unsigned int funct3 = (op & (7 << 12)) >> 12 ;
            if (funct3 == 0) {
                cout << "sb" << endl ;
                return sb ;
            } else if (funct3 == 1) {
                cout << "sh" << endl ;
                return sh ;
            } else if (funct3 == 2) {
                cout << "sw" << endl ;
                return sw ;
            }
        } else if (opcode == 19) {
            cout << "I Type" << endl ;
            unsigned int funct3 = (op & (7 << 12)) >> 12 ;
            unsigned int funct7 = (op & (255u << 25)) >> 25 ;
            if (funct3 == 0) {
                cout << "addi" << endl ;
                return addi ;
            } else if (funct3 == 1) {
                if (funct7 == 0) {
                    cout << "slli" << endl ;
                    return slli ;
                }
            } else if (funct3 == 2) {
                cout << "slti" << endl ;
                return slti ;
            } else if (funct3 == 3) {
                cout << "sltiu" << endl ;
                return sltiu ;
            } else if (funct3 == 4) {
                cout << "xori" << endl ;
                return xori ;
            } else if (funct3 == 5) {
                if (funct7 == 0) {
                    cout << "srli" << endl ; 
                    return srli ;
                } else if (funct7 == 32) {
                    cout << "srai" << endl ;
                    return srai ;
                }
            } else if (funct3 == 6) {
                cout << "ori" << endl ;
                return ori ;
            } else if (funct3 == 7) {
                cout << "andi" << endl ;
                return andi ;
            }
        } else if (opcode == 51) {
            cout << "R Type" << endl ;
            unsigned int funct3 = (op & (7 << 12)) >> 12 ;
            unsigned int funct7 = (op & (255u << 25)) >> 25 ;
            if (funct3 == 0) {
                if (funct7 == 0) {
                    cout << "add" << endl ;
                    return add ;
                } else if (funct7 == 32) {
                    cout << "sub" << endl ;
                    return sub ;
                }
            } else if (funct3 == 1) {
                cout << "sll" << endl ;
                return sll ;
            } else if (funct3 == 2) {
                cout << "slt" << endl ;
                return slt ;
            } else if (funct3 == 3) {
                cout << "sltu" << endl ;
                return sltu ;
            } else if (funct3 == 4) {
                cout << "xor" << endl ;
                return _xor ;
            } else if (funct3 == 5) {
                if (funct7 == 0) {
                    cout << "srl" << endl ;
                    return srl ;
                } else if (funct7 == 32) {
                    cout << "sra" << endl ;
                    return sra ;
                }
            } else if (funct3 == 6) {
                cout << "or" << endl ;
                return _or ;
            } else if (funct3 == 7) {
                cout << "and" << endl ;
                return  _and ;
            }
        }
    }
} ;

#endif