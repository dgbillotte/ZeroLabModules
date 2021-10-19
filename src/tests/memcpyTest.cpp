#include <iostream>


// int DUMP = true;
// #include "dump.hpp"

void d(int* buf) {
    std::cout << "[";
    for(size_t i=0; i<=sizeof(*buf); i++) {
        std::cout << buf[i] << ", ";
    }
    std::cout << "]" << std::endl;
}


int main() {
    // int* buf = new int[5];

    int buf1[5] = {1,2,3,4,5};
    int* buf1p = buf1;
    int buf2[5] = {6,7,8,9,10};
    int* buf2p = buf2;

    d(buf1p);
    d(buf2p);

    memcpy(&(buf2p[1]), &(buf1p[2]), 2*sizeof(int));

    d(buf1p);
    d(buf2p);




}