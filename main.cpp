#include <bits/stdc++.h>
#include <sstream>

#include "decode.hpp"

using namespace std ;

unsigned int pc ;
unsigned int reg[32] ;
unsigned char memory[100000] ;

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
                for (int i = 0; i < 4; i ++) memory[pos + i] = a[i] ;
                pos += 4 ;
            }
        }
    }
}
void run () {
    pc = 0 ;
    unsigned op = (memory[pc + 3] << 24) + (memory[pc + 2] << 16) + (memory[pc + 1] << 8) + memory[pc] ;
    decode _decode = decode (op) ;
    optype _optype = _decode.decode_op() ;
}
int main() {
    input () ;
    run () ;
    return 0 ;
}