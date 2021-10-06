#ifndef DUMP_HPP
#define DUMP_HPP

#include <iostream>
#include "DelayBuffer.hpp"
#include "AllPassFilter.hpp"

int __d(int i) {
    if(DUMP)
        std::cout << "int: " << i << std::endl;
    return i;
}
float __d(float i) {
    if(DUMP)
        std::cout << "float: " << i << std::endl;
    return i;
}

void __d(DelayBuffer<float>& buf) {
    if(DUMP)
        buf.dump();
}

void __d(NestedAllPassFilter& buf) {
    if(DUMP)
        buf.dump();
}

#endif