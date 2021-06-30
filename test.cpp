#include <bits/stdc++.h>
#include <sstream>

using namespace std ;

unsigned int pc ;
unsigned int reg[32] ;
unsigned char memory[100000] ;

enum optype {
    lui, auipc, jal, jalr, 
    beq, bne, blt, bge, bltu, bgeu, 
    lb, lh, lw, lbu, lhu, 
    sb, sh, sw, 
    addi, slti, sltiu, xori, ori, andi, slli, srli, srai, 
    add, sub, sll, slt, sltu, _xor, srl, sra, _or, _and
} ;

optype decode_op (unsigned int op) {
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
void input () {
    freopen("sample.data", "r", stdin) ;
    unsigned int pos ;
    string s ;
    while (getline (cin, s)) {
        if (s[0] == '@') {
            stringstream sstream (s.substr (1)) ;
            sstream >> hex >> pos ;
        } else {
            stringstream sstream (s) ;
            unsigned int a[4] ;
            while (sstream >> hex >> a[0] >> a[1] >> a[2] >> a[3]) {
                reverse (a, a + 4) ;
                for (int i = 0; i < 4; i ++) memory[pos + i] = a[i] ;
                pos += 4 ;
                // unsigned op = (a[0] << 24) + (a[1] << 16) + (a[2] << 8) + a[3] ;
            }
        }
    }
}
void run () {
    pc = 0 ;
    while (1) {
        unsigned int op = ((unsigned int)memory[pc] << 24) + ((unsigned int)memory[pc + 1] << 16) + ((unsigned int)memory[pc + 2] << 8) + (unsigned int)memory[pc + 3] ;
        optype _optype = decode_op (op) ;
        
    }
}
int main() {
    input () ;
    run () ;
    return 0 ;
}