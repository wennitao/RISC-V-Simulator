#include "runcode.hpp"
#include "InstructionQueue.hpp"
#include "ReorderBuffer.hpp"
#include "RegisterStatus.hpp"
#include "ReservationStation.hpp"

extern unsigned int reg[32] ;
extern unsigned char memory[1000000] ;
extern RegisterStatus registerStatus_pre[32], registerStatus_next[32] ;
extern InstructionQueue instructionQueue_pre, instructionQueue_next ;
extern ReorderBuffer reorderBuffer_pre, reorderBuffer_next ;
extern ReservationStation reservationStation_pre, reservationStation_next ;