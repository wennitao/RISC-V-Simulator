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
std::vector<RS> RS_insert ;
std::vector<LSBuffer> LSB_insert ;
std::vector<pair<int, int> > RStatus_insert ;
std::vector<pair<int, unsigned int> > RStatus_update ;
std::vector<RS> Execute_ops ;
std::vector<ROB> ROB_commit ;
std::vector<pair<int, unsigned int> > CDB ;

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
        operation_parameter parameter = decode_op (op) ;
        parameter.pc = pc ;
        instructionQueue_next.push (parameter) ;
        pc += 4 ;
    }

    void run_rob() {
        for (int i = 0; i < ROB_insert.size(); i ++) {
            reorderBuffer_next.push (ROB_insert[i]) ;
            printf("ROB insert "); ROB_insert[i].print() ;
        }
        for (int i = 0; i < CDB.size(); i ++) {
            pair<int, unsigned int> cur = CDB[i] ;
            reorderBuffer_next.update (cur.first, cur.second) ;
            printf("ROB update %d %u\n", cur.first, cur.second) ;
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
            printf("RS insert "); RS_insert[i].print() ;
            reservationStation_next.push (RS_insert[i]) ;
        }
        for (int i = 0; i < 10; i ++) {
            if (!reservationStation_next.empty ()) {
                RS cur = reservationStation_next.front () ;
                if (cur.qj == -1 && cur.qk == -1) {
                    Execute_ops.push_back (cur) ;
                    reservationStation_next.pop() ;
                }
            }
        }
        for (int i = 0; i < CDB.size(); i ++) {
            pair<int, unsigned int> cur = CDB[i] ;
            printf("RS update %d %d\n", cur.first, cur.second) ;
            reservationStation_next.update (cur.first, cur.second) ;
        }
    }

    void run_RegisterStatus () {
        for (int i = 0; i < RStatus_insert.size(); i ++) {
            pair<int, int> cur = RStatus_insert[i] ;
            registerStatus_next[cur.first].q = cur.second ;
            registerStatus_next[cur.first].busy = true ;
        }
        for (int i = 0; i < RStatus_update.size(); i ++) {
            pair<int, unsigned int> cur = RStatus_update[i] ;
            registerStatus_next[cur.first].q = -1 ;
            registerStatus_next[cur.first].busy = false ;
        }
    }

    void run_lsbuffer () {
        for (int i = 0; i < LSB_insert.size(); i ++) {
            printf("LSB insert "); LSB_insert[i].print() ;
            loadStoreBuffer_next.push (LSB_insert[i]) ;
        }
        if (!loadStoreBuffer_next.empty()) {
            LSBuffer cur = loadStoreBuffer_next.front() ;
            if (cur.qj == -1 && cur.qk == -1) {
                loadStoreBuffer_next.pop() ;
                if (cur.store == 0) {
                    unsigned int address = cur.vj + cur.A ;
                    unsigned int result = memory[address] ;
                    CDB.push_back (make_pair (cur.dest, result)) ;
                    printf("LSB load %u %u\n", address, result) ;
                } else {
                    unsigned int address = cur.vj + cur.A ;
                    unsigned int result = cur.vk ;
                    memory[address] = result ;
                    printf("LSB store %u %u\n", address, result) ;
                }
            }
        }
        for (int i = 0; i < CDB.size(); i ++) {
            pair<int, unsigned int> cur = CDB[i] ;
            printf("LSB update %d %u\n", cur.first, cur.second) ;
            loadStoreBuffer_next.update (cur.first, cur.second) ;
        }
    }

    void update () {
        ROB_insert.clear() ;
        RS_insert.clear() ;
        LSB_insert.clear() ;
        CDB.clear() ;
        RStatus_insert.clear() ;
        RStatus_update.clear() ;
        Execute_ops.clear() ;
        ROB_commit.clear() ;

        for (int i = 0; i < 32; i ++)
            registerStatus_pre[i] = registerStatus_next[i] ;
        instructionQueue_pre = instructionQueue_next ;
        reorderBuffer_pre = reorderBuffer_next ;
        reservationStation_pre = reservationStation_next ;
        loadStoreBuffer_pre = loadStoreBuffer_next ;
    }

    void run_issue () {
        if (reorderBuffer_next.full()) return ;
        printf("issue\n") ;
        operation_parameter op = instructionQueue_next.front(); instructionQueue_next.pop() ;
        op.print() ;
        ROB_insert.push_back (op) ;
        int cur_ROB_pos = reorderBuffer_pre.nextPos() ;
        if (op.rd != 0 && op.TYPE != 'S') {
            RStatus_insert.push_back (make_pair (op.rd, cur_ROB_pos)) ;     
        }
        switch (op.type) {
            case lui: {
                CDB.push_back (make_pair (cur_ROB_pos, op.imm)) ;
                break ;
            }
            case auipc: {
                RS cur; cur.vj = op.pc; cur.vk = op.imm; cur.dest = cur_ROB_pos; cur.op = auipc ;
                RS_insert.push_back (cur) ;
                break ;
            }
            case jal: {
                RS cur; cur.vj = op.pc; cur.vk = op.imm; cur.dest = cur_ROB_pos; cur.op = jal ;
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
                RS cur ;
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
                RS cur ;
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
                RS cur ;
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
                RS cur ;
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
                        cur.qk = 0 ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos ;
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
            case lbu: {
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 0; cur.dest = cur_ROB_pos ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 1 ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 1 ;
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
                LSBuffer cur; cur.A = op.imm; cur.store = 1 ; 
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
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = slli ;
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
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = srli ;
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
                RS cur; cur.vk = op.imm; cur.qk = -1; cur.op = srai ;
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
                        cur.qj = 0 ;
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
            case sub: {
                RS cur; cur.op = sub ;
                if (registerStatus_pre[op.rs].busy) {
                    int h = registerStatus_pre[op.rs].q ;
                    if (reorderBuffer_pre.que[h].ready) {
                        cur.vj = reorderBuffer_pre.que[h].value ;
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
                        cur.qj = 0 ;
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
    }

    void execute () {
        for (int i = 0; i < Execute_ops.size(); i ++) {
            printf("execute\n") ;
            RS rs = Execute_ops[i] ;
            unsigned int result ;
            switch (rs.op) {
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
            int d = cur.dest ;
            if (cur.instruction == 'B') {
                switch (cur.op) {
                    case jal: {
                        reg[d] = cur.pc + 4 ;
                        RStatus_update.push_back (make_pair (d, cur.pc + 4)) ;
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    case jalr: {
                        reg[d] = cur.pc + 4 ;
                        RStatus_update.push_back (make_pair (d, cur.pc + 4)) ;
                        if (cur.value != pc + 4) clear(), pc = cur.value ;
                        break ;
                    }
                    case beq: {
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    case bne: {
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    case blt: {
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    case bge: {
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    case bltu: {
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    case bgeu: {
                        if (cur.value != 4) clear(), pc += cur.value ;
                        break ;
                    }
                    default:
                        break ;
                }
            } else if (cur.instruction == 'E') {
                throw "return" ;
            } else {
                reg[d] = cur.value ;
                RStatus_update.push_back (make_pair (d, cur.value)) ;
            }
        }
    }

    void run () {
        unsigned int clock = 0 ;
        while (1) {
            printf("clock:%d pc:%x\n", clock ++, pc) ;
            run_rob() ;
            run_lsbuffer() ;
            run_reservation() ;
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