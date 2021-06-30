#include <bits/stdc++.h>
#include <sstream>

#include "decode.hpp"
#include "runcode.hpp"

using namespace std ;

unsigned int pc ;
unsigned int reg[32] ;
unsigned char memory[100000] ;

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
    pc = 0 ;
    while (1) {
        unsigned op = (memory[pc + 3] << 24) + (memory[pc + 2] << 16) + (memory[pc + 1] << 8) + memory[pc] ;
        if (op == 0) break ;
        decode _decode = decode (op) ;
        operation_parameter parameter = _decode.decode_op () ;
        runcode _runcode = runcode (parameter) ;
    }
}
int main() {
    input () ;
    run () ;
    return 0 ;
}