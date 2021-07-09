#ifndef RISCV_runcode
#define RISCV_runcode

// #define debug
// #define compare

#include <bits/stdc++.h>

#include "main.h"
#include "decode.hpp"
#include "InstructionQueue.hpp"
#include "ReorderBuffer.hpp"
#include "RegisterStatus.hpp"
#include "ReservationStation.hpp"
#include "LoadStoreBuffer.hpp"

std::vector<ROB> ROB_insert ;
std::vector<RS> RS_insert ;
std::vector<LSBuffer> LSB_insert ;
std::vector<pair<int, int> > RStatus_insert ;
std::vector<pair<int, int> > RStatus_update ;
std::vector<RS> Execute_ops ;
std::vector<ROB> ROB_commit ;
std::vector<pair<int, unsigned int> > CDB, CDB_next ;

class runcode {
private:
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
        if (op == 0) return ;
        operation_parameter parameter = decode_op (op) ;
        parameter.pc = pc ;
        instructionQueue_next.push (parameter) ;
        pc += 4 ;
    }

    void run_rob() {
        for (int i = 0; i < ROB_insert.size(); i ++) {
            reorderBuffer_next.push (ROB_insert[i]) ;
            #ifdef debug
                printf("ROB insert "); ROB_insert[i].print() ;
            #endif
        }
        for (int i = 0; i < CDB.size(); i ++) {
            pair<int, unsigned int> cur = CDB[i] ;
            reorderBuffer_next.update (cur.first, cur.second) ;
            #ifdef debug
                printf("ROB update %d %u\n", cur.first, cur.second) ;
            #endif
        }
        if (reorderBuffer_next.empty()) return ;
        ROB cur = reorderBuffer_next.front() ;
        if (cur.ready == true) {
            ROB_commit.push_back (cur) ;
            reorderBuffer_next.pop() ;
        }
    }

    void run_reservation() {
        for (int i = 0; i < RS_insert.size(); i ++) {
            #ifdef debug
                printf("RS insert "); RS_insert[i].print() ;
            #endif
            reservationStation_next.push (RS_insert[i]) ;
        }
        if (!reservationStation_next.empty ()) {
            RS cur = reservationStation_next.front () ;
            if (cur.qj == -1 && cur.qk == -1) {
                Execute_ops.push_back (cur) ;
                reservationStation_next.pop() ;
            }
        }
        for (int i = 0; i < CDB.size(); i ++) {
            pair<int, unsigned int> cur = CDB[i] ;
            #ifdef debug
                printf("RS update %d %d\n", cur.first, cur.second) ;
            #endif
            reservationStation_next.update (cur.first, cur.second) ;
        }
    }

    void run_RegisterStatus () {
        for (int i = 0; i < RStatus_insert.size(); i ++) {
            pair<int, int> cur = RStatus_insert[i] ;
            // if (cur.first == 15) printf("RStatus 15 ROBID:%d busy\n", cur.second) ;
            registerStatus_next[cur.first].q = cur.second ;
            registerStatus_next[cur.first].busy = true ;
        }
        for (int i = 0; i < RStatus_update.size(); i ++) {
            pair<int, int> cur = RStatus_update[i] ;
            int d = cur.first, ROB_id = cur.second ;
            if (registerStatus_next[d].q == ROB_id) {
                // printf("RStatus 15 ROBID:%d free\n", cur.second) ;
                registerStatus_next[d].q = -1 ;
                registerStatus_next[d].busy = false ;
            }
        }
    }

    void run_lsbuffer () {
        for (int i = 0; i < LSB_insert.size(); i ++) {
            #ifdef debug
                printf("LSB insert "); LSB_insert[i].print() ;
            #endif
            loadStoreBuffer_next.push (LSB_insert[i]) ;
        }
        if (!loadStoreBuffer_next.empty()) {
            LSBuffer cur = loadStoreBuffer_next.front() ;
            if (cur.qj == -1 && cur.qk == -1) {
                if (cur.store == 0) {
                    loadStoreBuffer_next.pop() ;
                    unsigned int address = cur.vj + cur.A ;
                    unsigned int result ;
                    if (cur.op == lb) result = sext (memory[address], 7) ;
                    else if (cur.op == lh) {
                        result = ((unsigned int)memory[address + 1] << 8) + memory[address] ;
                        result = sext (result, 15) ;
                    } else if (cur.op == lw) {
                        result = ((unsigned int)memory[address + 3] << 24) + ((unsigned int)memory[address + 2] << 16) + ((unsigned int)memory[address + 1] << 8) + memory[address] ;
                    } else if (cur.op == lbu) {
                        result = memory[address] ;
                    } else if (cur.op == lhu) {
                        result = ((unsigned int)memory[address + 1] << 8) + memory[address] ;
                    }
                    CDB_next.push_back (make_pair (cur.dest, result)) ;
                    #ifdef debug
                        printf("LSB load from:%u result:%u\n", address, result) ;
                    #endif
                } else {
                    if (cur.commit) {
                        loadStoreBuffer_next.pop() ;
                        unsigned int address = cur.vj + cur.A ;
                        unsigned int result = cur.vk ;
                        if (cur.op == sb) result = result & ((1 << 8) - 1) ;
                        else if (cur.op == sh) result = result & ((1 << 16) - 1) ;
                        memory[address] = result & ((1 << 8) - 1) ;
                        memory[address + 1] = (result >> 8) & ((1 << 8) - 1) ;
                        memory[address + 2] = (result >> 16) & ((1 << 8) - 1) ;
                        memory[address + 3] = (result >> 24) ;
                        #ifdef compare
                            printf("store to address:%u value:%u\n", address, result) ;
                        #endif
                        #ifdef debug
                            printf("********** LSB store %u %u **********\n", address, result) ;
                        #endif
                    } else {
                        #ifdef debug
                            printf("LSB update commit: pos %d\n", cur.dest) ;
                        #endif
                        reorderBuffer_next.update (cur.dest, 0) ;
                    }
                }
            }
        }
        for (int i = 0; i < CDB.size(); i ++) {
            pair<int, unsigned int> cur = CDB[i] ;
            #ifdef debug
                printf("LSB update %d %u\n", cur.first, cur.second) ;
            #endif
            loadStoreBuffer_next.update (cur.first, cur.second) ;
        }
    }

    void update () {
        ROB_insert.clear() ;
        RS_insert.clear() ;
        LSB_insert.clear() ;
        CDB = CDB_next; CDB_next.clear() ;
        RStatus_insert.clear() ;
        RStatus_update.clear() ;

        for (int i = 0; i < 32; i ++)
            registerStatus_pre[i] = registerStatus_next[i] ;
        instructionQueue_pre = instructionQueue_next ;
        reorderBuffer_pre = reorderBuffer_next ;
        reservationStation_pre = reservationStation_next ;
        loadStoreBuffer_pre = loadStoreBuffer_next ;
    }

    void run_issue () {
        if (instructionQueue_next.empty()) return ;
        if (reorderBuffer_next.full()) return ;
        if (reservationStation_next.full()) return ;
        if (loadStoreBuffer_next.full()) return ;
        operation_parameter op = instructionQueue_next.front(); instructionQueue_next.pop() ;
        #ifdef debug
            printf("issue "); op.print() ;
        #endif
        int cur_ROB_pos = reorderBuffer_next.nextPos(), LSB_pos ;
        // printf("issue insert ROB pos:%d\n", cur_ROB_pos) ;
        if (op.rd != 0 && op.TYPE != 'S' && op.TYPE != 'E') {
            RStatus_insert.push_back (make_pair (op.rd, cur_ROB_pos)) ;     
        }
        switch (op.type) {
            case lui: {
                RS cur; cur.vj = 0; cur.vk = op.imm; cur.dest = cur_ROB_pos; cur.op = lui ;
                cur.busy = true ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case auipc: {
                RS cur; cur.vj = op.pc; cur.vk = op.imm; cur.dest = cur_ROB_pos; cur.op = auipc ;
                cur.busy = true ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case jal: {
                RS cur; cur.vj = op.pc; cur.vk = op.imm; cur.dest = cur_ROB_pos; cur.op = jal ;
                cur.busy = true ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case jalr: {
                RS cur; cur.dest = cur_ROB_pos; cur.vk = op.imm; cur.op = jalr ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                RS_insert.push_back (cur) ;
                break ;
            }
            case beq: {
                RS cur; cur.imm = op.imm ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos; cur.op = beq ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case bne: {
                RS cur; cur.imm = op.imm ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos; cur.op = bne ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case blt: {
                RS cur; cur.imm = op.imm ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos; cur.op = blt ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case bge: {
                RS cur; cur.imm = op.imm ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos; cur.op = bge ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case bltu: {
                RS cur; cur.imm = op.imm ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos; cur.op = bltu ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case bgeu: {
                RS cur; cur.imm = op.imm ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos; cur.op = bgeu ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case lb: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos; cur.op = lb ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case lh: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos; cur.op = lh ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case lw: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos; cur.op = lw ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                // printf("issue "); cur.print() ;
                LSB_insert.push_back (cur) ;
                break ;
            }
            case lbu: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos; cur.op = lbu ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case lhu: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos; cur.op = lhu ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case sb: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 1; cur.op = sb; cur.dest = cur_ROB_pos ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case sh: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 1; cur.op = sh; cur.dest = cur_ROB_pos ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case sw: {
                LSB_pos = loadStoreBuffer_next.nextPos() ;
                LSBuffer cur; cur.A = op.imm; cur.store = 1; cur.op = sw; cur.dest = cur_ROB_pos ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                LSB_insert.push_back (cur) ;
                break ;
            }
            case addi: {
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = addi ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case slti: {
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = slti ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case sltiu: {
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = sltiu ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case xori: {
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = xori ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case ori: {
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = ori ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case andi: {
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = andi ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case slli: {
                RS cur; cur.vk = op.shamt; cur.qk = -1; cur.op = slli ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case srli: {
                RS cur; cur.vk = op.shamt; cur.qk = -1; cur.op = srli ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case srai: {
                RS cur; cur.vk = op.shamt; cur.qk = -1; cur.op = srai ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case add: {
                RS cur; cur.op = add ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                // printf("ROBID:%d qj:%d qk:%d\n", registerStatus_pre[op.rs].q, cur.qj, cur.qk) ;
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case sub: {
                RS cur; cur.op = sub ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case sll: {
                RS cur; cur.op = sll ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case slt: {
                RS cur; cur.op = slt ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case sltu: {
                RS cur; cur.op = sltu ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case _xor: {
                RS cur; cur.op = _xor ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case srl: {
                RS cur; cur.op = srl ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case sra: {
                RS cur; cur.op = sra ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case _or: {
                RS cur; cur.op = _or ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case _and: {
                RS cur; cur.op = _and ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = -1 ;
                    } else {
                        cur.qj = h ;
                    }
                } else {
                    cur.vj = reg[op.rs]; cur.qj = -1 ;
                }
                if (registerStatus_pre[op.rt].busy) {
                    int h = registerStatus_pre[op.rt].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vk = reorderBuffer_pre.que[h].value ;
                        cur.qk = -1 ;
                    } else {
                        cur.qk = h ;
                    }
                } else {
                    cur.vk = reg[op.rt]; cur.qk = -1 ;
                }
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (cur) ;
                break ;
            }
            default:
                break ;
        }
        ROB cur ;
        cur.instruction = op.TYPE; cur.dest = op.rd; cur.ready = false ;
        cur.op = op.type; cur.pc = op.pc; cur.LSBuffer_id = LSB_pos ;
        if (cur.instruction == 'E') cur.ready = true ;
        ROB_insert.push_back (cur) ;
    }

    void execute () {
        for (int i = 0; i < Execute_ops.size(); i ++) {
            RS rs = Execute_ops[i] ;
            #ifdef debug
                printf("execute "); rs.print() ;
            #endif
            unsigned int result ;
            switch (rs.op) {
                case lui: {
                    result = rs.vj + rs.vk ;
                }
                case auipc: {
                    result = rs.vj + rs.vk ;
                    break ;
                }
                case jal: {
                    result = rs.vk ;
                    break ;
                }
                case jalr: {
                    result = (rs.vj + rs.vk) & ~1 ;
                    // printf("execute jalr result %u\n", result) ;
                    break ;
                }
                case beq: {
                    if (rs.vj == rs.vk) result = rs.imm ;
                    else result = 4 ;
                    break ;
                }
                case bne: {
                    if (rs.vj != rs.vk) result = rs.imm ;
                    else result = 4 ;
                    break ;
                }
                case blt: {
                    if ((int)rs.vj < (int)rs.vk) result = rs.imm ;
                    else result = 4 ;
                    break ;
                }
                case bge: {
                    if ((int)rs.vj >= (int)rs.vk) result = rs.imm ;
                    else result = 4 ;
                    break ;
                }
                case bltu: {
                    if (rs.vj < rs.vk) result = rs.imm ;
                    else result = 4 ;
                    break ;
                }
                case bgeu: {
                    if (rs.vj >= rs.vk) result = rs.imm ;
                    else result = 4 ;
                    break ;
                }
                case addi: {
                    result = rs.vj + rs.vk ;
                    break ;
                }
                case slti: {
                    result = (int)rs.vj < (int)rs.vk ;
                    break ;
                }
                case sltiu: {
                    result = rs.vj < rs.vk ;
                    break ;
                }
                case xori: {
                    result = rs.vj ^ rs.vk ;
                    break ;
                }
                case ori: {
                    result = rs.vj | rs.vk ;
                    break ;
                }
                case andi: {
                    result = rs.vj & rs.vk ;
                    break ;
                }
                case slli: {
                    result = rs.vj << rs.vk ;
                    break ;
                }
                case srli: {
                    result = rs.vj >> rs.vk ;
                    break ;
                }
                case srai: {
                    result = sext (rs.vj >> rs.vk, 31 - rs.vk) ;
                    break ;
                }
                case add: {
                    result = rs.vj + rs.vk ;
                    break ;
                }
                case sub: {
                    result = rs.vj - rs.vk ;
                    break ;
                }
                case sll: {
                    result = rs.vj << (rs.vk & ((1 << 5) - 1)) ;
                    break ;
                }
                case slt: {
                    result = (int)rs.vj < (int)rs.vk ;
                    break ;
                }
                case sltu: {
                    result = rs.vj < rs.vk ;
                    break ;
                }
                case _xor: {
                    result = rs.vj ^ rs.vk ;
                    break ;
                }
                case srl: {
                    result = rs.vj >> (rs.vk & ((1 << 5) - 1)) ;
                    break ;
                }
                case sra: {
                    result = sext (rs.vj >> (rs.vk & ((1 << 5) - 1)), 31 - (rs.vk & ((1 << 5) - 1))) ;
                    break ;
                }
                case _or: {
                    result = rs.vj | rs.vk ;
                    break ;
                }
                case _and: {
                    result = rs.vj & rs.vk ;
                    break ;
                }
                default:
                    break;
            }
            CDB.push_back (make_pair (rs.dest, result)) ;
        }
        Execute_ops.clear() ;
    }

    void clear() {
        ROB_insert.clear() ;
        RS_insert.clear() ;
        LSB_insert.clear() ;
        CDB.clear() ;
        RStatus_insert.clear() ;
        RStatus_update.clear() ;
        Execute_ops.clear() ;

        instructionQueue_next.clear() ;
        reorderBuffer_next.clear() ;
        reservationStation_next.clear() ;
        loadStoreBuffer_next.clear() ;

        for (int i = 0; i < 32; i ++) {
            registerStatus_next[i].busy = false ;
            registerStatus_next[i].q = -1 ;
        }
    }

    void commit () {
        for (int i = 0; i < ROB_commit.size(); i ++) {
            ROB cur = ROB_commit[i] ;
            #ifdef debug
                printf("commit "); cur.print() ;
            #endif
            int d = cur.dest ;
            if (cur.instruction == 'B') {
                switch (cur.op) {
                    case jal: {
                        reg[d] = cur.pc + 4 ;
                        RStatus_update.push_back (make_pair (d, cur.ROB_id)) ;
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    case jalr: {
                        reg[d] = cur.pc + 4 ;
                        RStatus_update.push_back (make_pair (d, cur.ROB_id)) ;
                        if (cur.value != cur.pc + 4) clear(), pc = cur.value ;
                        break ;
                    }
                    case beq: {
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    case bne: {
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    case blt: {
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    case bge: {
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    case bltu: {
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    case bgeu: {
                        if (cur.value != 4) clear(), pc = cur.pc + cur.value ;
                        break ;
                    }
                    default:
                        break ;
                }
            } else if (cur.instruction == 'E') {
                throw "return" ;
            } else if (cur.instruction == 'S') {
                loadStoreBuffer_next.update_commit (cur.LSBuffer_id) ;
            } else {
                #ifdef debug
                    printf("reg update %d %u\n", d, cur.value) ;
                #endif
                reg[d] = cur.value ;
                RStatus_update.push_back (make_pair (d, cur.ROB_id)) ;
            }
        }
        reg[0] = 0 ;
        #ifdef compare
            if (!ROB_commit.empty()) {
                printf("reg:") ;
                for (int i = 0; i < 32; i ++) printf("%u ", reg[i]) ;
                printf("\n") ;
            }
        #endif
        ROB_commit.clear() ;
    }

    void run () {
        unsigned int clock = 0 ;
        while (1) {
            #ifdef debug
                printf("\nclock:%d pc:%x\n", clock ++, pc) ;
            #endif
            run_rob() ;
            #ifdef debug
                reorderBuffer_next.print() ;
            #endif
            run_lsbuffer() ;
            run_reservation() ;
            #ifdef debug
                reservationStation_next.print() ;
            #endif
            run_RegisterStatus() ;
            run_inst_fetch_queue() ;
            update () ;

            execute() ;
            run_issue() ;
            commit() ;
        }
    }
} ;

#endif