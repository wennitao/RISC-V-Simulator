#ifndef RISCV_runcode
#define RISCV_runcode

#include <bits/stdc++.h>

#include "main.h"
#include "decode.hpp"
#include "InstructionQueue.hpp"
#include "ReorderBuffer.hpp"
#include "RegisterStatus.hpp"
#include "ReservationStation.hpp"
#include "LoadStoreBuffer.hpp"

std::vector<operation_parameter> ROB_insert ;
std::vector<pair<int, int> > ROB_update ;
std::vector<pair<RS_op, RS> > RS_insert ;
std::vector<pair<bool, LSBuffer> > LSB_insert ;

class runcode {
private:
    unsigned int result, store_pos ;
    unsigned int pc = 0 ;

public:
    runcode () {}

    unsigned int sext (unsigned int x, int bit) {
        if ((x >> bit) & 1) {
            unsigned int tmp = ~((1u << (bit + 1)) - 1) ;
            x = x | tmp ;
        }
        return x ;
    }

    void run_inst_fetch_queue() {
        if (instructionQueue_next.full()) return ;
        unsigned int op = ((unsigned int)memory[pc + 3] << 24) + ((unsigned int)memory[pc + 2] << 16) + ((unsigned int)memory[pc + 1] << 8) + memory[pc] ;
        operation_parameter parameter = decode_op (op) ;
        parameter.pc = pc ;
        instructionQueue_next.push (parameter) ;
        pc += 4 ;
    }

    void run_rob() {

    }

    void run_reservation() {

    }

    void update () {
        ROB_insert.clear() ;
        ROB_update.clear() ;
        RS_insert.clear() ;
    }

    void run_issue () {
        if (reorderBuffer_next.full()) return ;
        operation_parameter op = instructionQueue_next.front(); instructionQueue_next.pop() ;
        ROB_insert.push_back (op) ;
        int cur_ROB_pos = reorderBuffer_pre.nextPos() ;
        if (op.rd != 0) registerStatus_pre[op.rd].q = cur_ROB_pos ;
        switch (op.type) {
            case lui: {
                ROB_update.push_back (make_pair (cur_ROB_pos, op.imm)) ;
                break ;
            }
            case auipc: {
                RS cur; cur.vj = op.pc; cur.vk = op.imm; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_add, cur)) ;
                break ;
            }
            case jal: {
                RS cur; cur.vj = op.pc; cur.vk = 4; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_add, cur)) ;
                break ;
            }
            case jalr: {
                
            }
            case beq: {
                RS cur ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = 0 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = 0 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = 0 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_equal, cur)) ;
            }
            case bne: {
                RS cur ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = 0 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = 0 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = 0 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_equal, cur)) ;
            }
            case blt: {
                RS cur ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = 0 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = 0 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = 0 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_less, cur)) ;
            }
            case bge: {
                RS cur ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = 0 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = 0 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = 0 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_less, cur)) ;
            }
            case bltu: {
                RS cur ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = 0 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = 0 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = 0 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_lessu, cur)) ;
            }
            case bgeu: {
                RS cur ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = 0 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = 0 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = 0 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_lessu, cur)) ;
            }
            case lb: {
                
            }
            default:
                break;
        }
    }

    void execute () {
        
    }

    void commit () {

    }

    void run () {

    }
} ;

#endif