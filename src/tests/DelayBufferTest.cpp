#include <iostream>
#include "DelayBuffer.hpp"
#include <assert.h>

int DUMP = true;
#include "dump.hpp"

int main() {

    // const size_t maxsize = 10;
    // const size_t size = 6;

    DelayBuffer<float> b1(3);
    b1.push(1);
    b1.push(2);
    b1.push(3);
    __d(b1);
    b1.push(b1.read());
    b1.push(b1.read());
    b1.push(b1.read());
    b1.push(b1.read());
    b1.push(b1.read());
    b1.push(b1.read());
    b1.push(b1.read());
    __d(b1);

    return 0;

    DelayBuffer<float> buf(10, 5);
    // DelayBuffer<float> buf(10, 4);
    // buf.dump();

    buf.push(91);
    buf.push(92);
    buf.push(93);
    __d(buf);
    assert(buf.read(0) == 93);
    assert(buf.read(1) == 92);
    assert(buf.read(2) == 91);
    buf.size(6);
    __d(buf);

    assert(buf.read(0) == 93);
    assert(buf.read(1) == 92);
    assert(buf.read(2) == 91);
    buf.push(94);
    buf.push(95);
    buf.push(96);
    buf.push(97);
    __d(buf);
    assert(buf.read() == 92);
    assert(buf.read(0) == 97);
    assert(buf.read(1) == 96);
    assert(buf.read(2) == 95);
    
    // buf.size(4);
    // __d(buf);
    // buf.clear(true);
    // __d(buf);

    buf.push(21);
    __d(buf);
    buf.size(9);
    __d(buf);
    assert(buf.read(0) == 21);
    // assert(buf.read() == 21);
    
    buf.push(22);
    buf.push(23);
    buf.push(24);
    buf.push(25);
    __d(buf);
    assert(buf.read(0) == 25);
    buf.size(10);
    assert(buf.read(0) == 25);
    __d(buf);

    buf.size(7);
    __d(buf);
    assert(buf.read(0) == 25);
    assert(buf.read() == 96);

    buf.push(26);
    buf.push(27);
    __d(buf);
    assert(buf.read(0) == 27);
    buf.size(4);
    __d(buf);
    assert(buf.read(0) == 27);
    assert(buf.read() == 24);


    // return 0;




    // Basic Circular Buffer Tests
    buf.size(5);
    buf.clear();
    buf.push(0);
    __d(buf);
    assert(buf.read(0) == 0);

    buf.size(3);

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
    assert(__d(buf.read(1)) == 8);
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
