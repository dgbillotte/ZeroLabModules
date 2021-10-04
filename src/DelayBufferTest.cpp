#include <iostream>
#include "DelayBuffer.hpp"

int xmain() {

    const size_t size = 3;


    // DelayBufferOld<float, size> buf;
    DelayBuffer<float> buf(size);

    buf.dump();

    for(int i=1; i<12; i++) {

        if(i % 5 == 0)
            buf.clear();

        // add an item
        buf.push(i*10);
        buf.dump();

        // get all of the items
        for(size_t j = 0; j < size; j++) {
            std::cout << "\tD(" << j << "): " << buf.read(j) << std::endl;
        }
    }

    return 0;
}
