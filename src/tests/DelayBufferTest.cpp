#include <iostream>
#include "DelayBuffer.hpp"
#include <assert.h>

int DUMP = false;
#include "dump.hpp"

int main() {

    const size_t size = 3;

    // Basic Circular Buffer Tests
    DelayBuffer<float> buf(size);
    // buf.dump();
    __d(buf);
    assert(buf.read(0) == 0);

    buf.push(3);
    buf.push(5);
    buf.push(8);

    __d(buf);
    // _head = 2
    assert(buf.read(0) == 8);
    assert(buf.read(1) == 5);
    assert(buf.read(2) == 3);

    buf.push(13);
    __d(buf);
    // _head = 0
    assert(buf.read(0) == 13);
    assert(buf.read(1) == 8);
    assert(buf.read(2) == 5);

    buf.push(21);
    __d(buf);
    // _head = 1
    assert(buf.read(0) == 21);
    assert(buf.read(1) == 13);
    assert(buf.read(2) == 8);    

    // Test out writes. There are several cases to test out here:
    // - test the middle and endpoints
    // - test for middle and endpoint positions of the _head pointer

    // __d(buf);
    // currently _head == 0
    buf.add(0, 11);
    __d(buf);
    assert(buf.read(0) == 32);
    assert(buf.read(1) == 13);
    assert(buf.read(2) == 8);
    
    buf.add(1, 11);
    __d(buf);
    assert(buf.read(0) == 32);
    assert(buf.read(1) == 24);
    assert(buf.read(2) == 8);
    
    buf.add(2, 11);
    __d(buf);
    assert(buf.read(0) == 32);
    assert(buf.read(1) == 24);
    assert(buf.read(2) == 19);

    std::cout << "Successfully Completed All Tests" << std::endl;

    return 0;
}
