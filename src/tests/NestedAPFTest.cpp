#include <iostream>
#include "DelayBuffer.hpp"
#include "AllPassFilter.hpp"
#include <assert.h>

int DUMP = true;
#include "dump.hpp"

int main() {

    const size_t size = 5;

    DelayBuffer<float> buf(size);
    int start = 0;
    int delay = 2;
    float g = 0.7f;
    NestedAllPassFilter apf(&buf, start, delay, g);

    buf.push(1.0);
    __d(apf);

    assert(__d(apf.process()) == -0.7f);
    assert(buf.read(start) == 1.0f+(g*-g));
    assert(buf.read(start+delay) == -g);

    // manually push the new value then call process
    buf.push(0.8);
    assert(apf.process() == -0.56f);
    __d(apf);

    // call process(float) which also does the push
    assert(apf.process(-0.3) == 0.72f);
    __d(apf);

    std::cout << "Successfully Completed All Tests" << std::endl;

    return 0;
}
