#include <bits/stdc++.h>
#include <sstream>

#include "decode.hpp"
#include "runcode.hpp"
#include "RegisterStatus.hpp"

using namespace std ;

unsigned int reg[32] ;
unsigned char memory[1000000] ;

RegisterStatus registerStatus_pre[32], registerStatus_next[32] ;
InstructionQueue instructionQueue_pre, instructionQueue_next ;
ReorderBuffer reorderBuffer_pre, reorderBuffer_next ;

void input () {
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
                for (int i = 0; i < 4; i ++) memory[pos + i] = a[i] ;
                pos += 4 ;
            }
        }
    }
}
void run () {
    runcode cpu ;
    cpu.run() ;
}
int main() {
    input () ;
    run () ;
    return 0 ;
}