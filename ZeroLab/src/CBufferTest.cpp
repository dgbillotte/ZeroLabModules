#include <iostream>
#include "CBuffer.hpp"

int main() {
    CBuffer buf(4);

    buf.dump();

    for(int i=1; i<12; i++) {
        buf.push(i*10);
        buf.dump();
    }
    
    for(int i=0; i<12; i++)
        std::cout << "idx " << i << ": " << buf[i] << std::endl;
}
