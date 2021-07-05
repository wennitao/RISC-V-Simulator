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
std::vector<pair<int, unsigned int> > ROB_update ;
std::vector<pair<RS_op, RS> > RS_insert ;
std::vector<pair<int, unsigned int> > RS_update ;
std::vector<pair<bool, LSBuffer> > LSB_insert ;
std::vector<pair<int, int> > RStatus_insert ;
std::vector<pair<RS_op, RS> > Execute_ops ;

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
            reorderBuffer_pre.push (ROB_insert[i]) ;
        }
        for (int i = 0; i < ROB_update.size(); i ++) {
            pair<int, unsigned int> cur = ROB_update[i] ;
            reorderBuffer_pre.update (cur.first, cur.second) ;
        }
        
    }

    void run_reservation() {
        for (int i = 0; i < RS_insert.size(); i ++) {
            reservationStation_pre.push (RS_insert[i]) ;
        }
        for (int i = 0; i < 10; i ++) {
            if (!reservationStation_pre.empty (i)) {
                RS cur = reservationStation_pre.front (i) ;
                if (cur.qj == -1 && cur.qk == -1) {
                    Execute_ops.push_back (make_pair (RS_ops[i], cur)) ;
                    reservationStation_pre.pop(i) ;
                }
            }
        }
    }

    void update () {
        ROB_insert.clear() ;
        ROB_update.clear() ;
        RS_insert.clear() ;
        LSB_insert.clear() ;
        RStatus_insert.clear() ;
        Execute_ops.clear() ;
    }

    void run_issue () {
        if (reorderBuffer_next.full()) return ;
        operation_parameter op = instructionQueue_next.front(); instructionQueue_next.pop() ;
        ROB_insert.push_back (op) ;
        int cur_ROB_pos = reorderBuffer_pre.nextPos() ;
        if (op.rd != 0) {
            RStatus_insert.push_back (make_pair (op.rd, cur_ROB_pos)) ;
        }
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
                RS_insert.push_back (make_pair (rs_equal, cur)) ;
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_equal, cur)) ;
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_less, cur)) ;
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_less, cur)) ;
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_lessu, cur)) ;
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_lessu, cur)) ;
                break ;
            }
            case lb: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (0, cur)) ;
                break ;
            }
            case lh: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (0, cur)) ;
                break ;
            }
            case lw: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (0, cur)) ;
                break ;
            }
            case lbu: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (0, cur)) ;
                break ;
            }
            case lhu: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (0, cur)) ;
                break ;
            }
            case sb: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (1, cur)) ;
                break ;
            }
            case sh: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (1, cur)) ;
                break ;
            }
            case sw: {
                LSBuffer cur; cur.A = op.imm ;
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
                LSB_insert.push_back (make_pair (1, cur)) ;
                break ;
            }
            case addi: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_add, cur)) ;
                break ;
            }
            case slti: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_less, cur)) ;
                break ;
            }
            case sltiu: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_lessu, cur)) ;
                break ;
            }
            case xori: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_xor, cur)) ;
                break ;
            }
            case ori: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_or, cur)) ;
                break ;
            }
            case andi: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_and, cur)) ;
                break ;
            }
            case slli: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_sll, cur)) ;
                break ;
            }
            case srli: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_srl, cur)) ;
                break ;
            }
            case srai: {
                RS cur; cur.vk = op.imm; cur.qk = -1 ;
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
                RS_insert.push_back (make_pair (rs_srl, cur)) ;
                break ;
            }
            case add: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_add, cur)) ;
                break ;
            }
            case sub: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_sub, cur)) ;
                break ;
            }
            case sll: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_sll, cur)) ;
                break ;
            }
            case slt: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_less, cur)) ;
                break ;
            }
            case sltu: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_lessu, cur)) ;
                break ;
            }
            case _xor: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_xor, cur)) ;
                break ;
            }
            case srl: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_srl, cur)) ;
                break ;
            }
            case sra: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_srl, cur)) ;
                break ;
            }
            case _or: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_or, cur)) ;
                break ;
            }
            case _and: {
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
                cur.busy = true; cur.dest = cur_ROB_pos ;
                RS_insert.push_back (make_pair (rs_and, cur)) ;
                break ;
            }
            default:
                break ;
        }
    }

    void execute () {
        for (int i = 0; i < Execute_ops.size(); i ++) {
            RS_op op = Execute_ops[i].first; RS cur_rs = Execute_ops[i].second ;
            unsigned int result ;
            switch (op) {
                case rs_add: {
                    result = cur_rs.vj + cur_rs.vk ;
                    break ;
                }
                case rs_sub: {
                    result = cur_rs.vj - cur_rs.vk ;
                    break ;
                }
                case rs_equal: {
                    result = cur_rs.vj == cur_rs.vk ;
                    break ;
                }
                case rs_less: {
                    result = (int)cur_rs.vj < (int)cur_rs.vk ;
                    break ;
                }
                case rs_lessu: {
                    result = cur_rs.vj < cur_rs.vk ;
                    break ;
                }
                case rs_xor: {
                    result = cur_rs.vj ^ cur_rs.vk ;
                    break ;
                }
                case rs_or: {
                    result = cur_rs.vj | cur_rs.vk ;
                    break ;
                }
                case rs_and: {
                    result = cur_rs.vj & cur_rs.vk ;
                    break ;
                }
                case rs_sll: {
                    result = cur_rs.vj << (cur_rs.vk & ((1 << 5) - 1)) ;
                    break ;
                }
                case rs_srl: {
                    result = cur_rs.vj >> (cur_rs.vk & ((1 << 5) - 1)) ;
                    break ;
                }
                default:
                    break;
            }
            ROB_update.push_back (make_pair (cur_rs.dest, result)) ;
            RS_update.push_back (make_pair (cur_rs.dest, result)) ;
        }
    }

    void commit () {

    }

    void run () {
        
    }
} ;

#endif