#ifndef RISCV_runcode
#define RISCV_runcode

#include <bits/stdc++.h>

#include "main.h"
#include "decode.hpp"

using namespace std;

class runcode {
private:
    operation_parameter op ;
    unsigned int result, store_pos ;

public:
    runcode () {}
    runcode (operation_parameter _op) : op (_op) {}

    unsigned int sext (unsigned int x, int bit) {
        if ((x >> bit) & 1) {
            unsigned int tmp = ~((1u << (bit + 1)) - 1) ;
            x = x | tmp ;
        }
        return x ;
    }

    void issue () {

    }

    void execute () {
        switch (op.type) {
            case lui: {
                // printf("lui %x %x\n", op.rd, op.imm) ;
                result = op.imm ;
                npc = pc + 4 ;
                break ;
            }
            case auipc: {
                // printf("auipc %x %x\n", op.rd, op.imm) ;
                result = pc + op.imm ;
                npc = pc + 4 ;
                break ;
            }
            case jal: {
                // printf("jal %x %x\n", op.rd, op.imm) ;
                result = pc + 4 ;
                npc += op.imm ;
                break ;
            }
            case jalr: {
                // printf("jalr %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                result = pc + 4 ;
                npc = (reg[op.rs] + op.imm) & ~1 ;
                break ;
            }
            case beq: {
                // printf("beq %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                if (reg[op.rs] == reg[op.rt]) npc = pc + op.imm ;
                else npc = pc + 4 ;
                break ;
            }
            case bne: {
                // printf("bne %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                if (reg[op.rs] != reg[op.rt]) npc = pc + op.imm ;
                else npc = pc + 4 ;
                break ;
            }
            case blt: {
                // printf("blt %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                if ((int)reg[op.rs] < (int)reg[op.rt]) npc = pc + op.imm ;
                else npc = pc + 4 ;
                break ;
            }
            case bge: {
                // printf("bge %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                if ((int)reg[op.rs] >= (int)reg[op.rt]) npc = pc + op.imm ;
                else npc = pc + 4 ;
                break ;
            }
            case bltu: {
                // printf("bltu %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                if (reg[op.rs] < reg[op.rt]) npc = pc + op.imm ;
                else npc = pc + 4 ;
                break ;
            }
            case bgeu: {
                // printf("bgeu %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                if (reg[op.rs] >= reg[op.rt]) npc = pc + op.imm ;
                else npc = pc + 4 ;
                break ;
            }
            case lb: {
                // printf("lb ~ lhu %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                unsigned int pos = reg[op.rs] + op.imm ;
                result = sext (memory[pos], 7) ;
                npc = pc + 4 ;
                break ;
            }
            case lh: {
                // printf("lb ~ lhu %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                unsigned int pos = reg[op.rs] + op.imm ;
                result = ((unsigned int)memory[pos + 1] << 8) + memory[pos] ;
                result = sext (result, 15) ;
                npc = pc + 4 ;
                break ;
            }
            case lw: {
                // printf("lb ~ lhu %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                unsigned int pos = reg[op.rs] + op.imm ;
                result = ((unsigned int)memory[pos + 3] << 24) + ((unsigned int)memory[pos + 2] << 16) + ((unsigned int)memory[pos + 1] << 8) + memory[pos] ;
                npc = pc + 4 ;
                break ;
            }
            case lbu: {
                // printf("lb ~ lhu %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                unsigned int pos = reg[op.rs] + op.imm ;
                result = memory[pos] ;
                npc = pc + 4 ;
                break ;
            }
            case lhu: {
                // printf("lb ~ lhu %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                unsigned int pos = reg[op.rs] + op.imm ;
                result = ((unsigned int)memory[pos + 1] << 8) + memory[pos] ;
                npc = pc + 4 ;
                break ;
            }
            case sb: {
                // printf("sb ~ sw %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                store_pos = reg[op.rs] + op.imm ;
                result = reg[op.rt] & ((1 << 8) - 1) ;
                npc = pc + 4 ;
                break ;
            }
            case sh: {
                // printf("sb ~ sw %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                store_pos = reg[op.rs] + op.imm ;
                result = reg[op.rt] & ((1 << 16) - 1) ;
                npc = pc + 4 ;
                break ;
            }
            case sw: {
                // printf("sb ~ sw %x %x %x\n", reg[op.rs], reg[op.rt], op.imm) ;
                store_pos = reg[op.rs] + op.imm ;
                result = reg[op.rt] ;
                npc = pc + 4 ;
                break ;
            }
            case addi: {
                // printf("addi %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                result = reg[op.rs] + op.imm ;
                npc = pc + 4 ;
                break ;
            }
            case slti: {
                // printf("slti %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                if ((int)reg[op.rs] < (int)op.imm) result = 1 ;
                else result = 0 ;
                npc = pc + 4 ;
                break ;
            }
            case sltiu: {
                // printf("sltiu %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                if (reg[op.rs] < op.imm) result = 1 ;
                else result = 0 ;
                npc = pc + 4 ;
                break ;
            }
            case xori: {
                // printf("xori %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                result = reg[op.rs] ^ op.imm ;
                npc = pc + 4 ;
                break ;
            }
            case ori: {
                // printf("ori %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                result = reg[op.rs] | op.imm ;
                npc = pc + 4 ;
                break ;
            }
            case andi: {
                // printf("andi %x %x %x\n", op.rd, reg[op.rs], op.imm) ;
                result = reg[op.rs] & op.imm ;
                npc = pc + 4 ;
                break ;
            }
            case slli: {
                // printf("slli %x %x %x\n", op.rd, reg[op.rs], op.shamt) ;
                if ((op.shamt) >> 5 != 0) break ;
                result = reg[op.rs] << op.shamt ;
                npc = pc + 4 ;
                break ;
            }
            case srli: {
                // printf("srli %x %x %x\n", op.rd, reg[op.rs], op.shamt) ;
                result = reg[op.rs] >> op.shamt ;
                npc = pc + 4 ;
                break ;
            }
            case srai: {
                // printf("srai %x %x %x\n", op.rd, reg[op.rs], op.shamt) ;
                result = sext (reg[op.rs] >> op.shamt, 31 - op.shamt) ;
                npc = pc + 4 ;
                break ;
            }
            case add: {
                // printf("add %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] + reg[op.rt] ;
                npc = pc + 4 ;
                break ;
            }
            case sub: {
                // printf("sub %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] - reg[op.rt] ;
                npc = pc + 4 ;
                break ;
            }
            case sll: {
                // printf("sll %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] << (reg[op.rt] & ((1 << 5) - 1)) ;
                npc = pc + 4 ;
                break ;
            }
            case slt: {
                // printf("slt %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                if ((int)reg[op.rs] < (int)reg[op.rt]) result = 1 ;
                else result = 0 ;
                npc = pc + 4 ;
                break ;
            }
            case sltu: {
                // printf("sltu %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                if (reg[op.rs] < reg[op.rt]) result = 1 ;
                else result = 0 ;
                npc = pc + 4 ;
                break ;
            }
            case _xor: {
                // printf("xor %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] ^ reg[op.rt] ;
                npc = pc + 4 ;
                break ;
            }
            case srl: {
                // printf("srl %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] >> (reg[op.rt] & ((1 << 5) - 1)) ;
                npc = pc + 4 ;
                break ;
            }
            case sra: {
                // printf("sra %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = sext (reg[op.rs] >> (reg[op.rt] & ((1 << 5) - 1)), 31 - (reg[op.rt] & ((1 << 5) - 1))) ;
                npc = pc + 4 ;
                break ;
            }
            case _or: {
                // printf("or %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] | reg[op.rt] ;
                npc = pc + 4 ;
                break ;
            }
            case _and: {
                // printf("and %x %x %x\n", op.rd, reg[op.rs], reg[op.rt]) ;
                result = reg[op.rs] & reg[op.rt] ;
                npc = pc + 4 ;
                break ;
            }
            default:
                break ;
        }
        pc = npc ;
    }

    void write_result () {
        // printf("pc:%x\n", pc) ;
        if (op.TYPE != 'B' && op.TYPE != 'S') {
            reg[op.rd] = result ;
        } else if (op.TYPE == 'S') {
            printf("store to address:%u value:%u\n", store_pos, result) ;
            memory[store_pos] = result & ((1 << 8) - 1) ;
            memory[store_pos + 1] = (result >> 8) & ((1 << 8) - 1) ;
            memory[store_pos + 2] = (result >> 16) & ((1 << 8) - 1) ;
            memory[store_pos + 3] = (result >> 24) ;
        }
        reg[0] = 0 ;
        for (int i = 0; i < 32; i ++) printf("%u ", reg[i]) ;
        printf("\n") ;
    }

    void run () {
        issue () ;
        execute () ;
        write_result () ;
    }
} ;

#endif