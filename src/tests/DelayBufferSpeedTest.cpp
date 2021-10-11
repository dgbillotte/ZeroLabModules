#include <iostream>
#include "DelayBuffer2.hpp"
// #include <cstdlib>
//#include <ctime>

#include <chrono>
using namespace std::chrono;


int main() {

    const size_t MAX_DELAY = 100000;
    const size_t NUM_LOOPS = 1000000;
    // Basic Circular Buffer Tests
    DelayBuffer2<float> buf(MAX_DELAY);
    // buf.dump();

    srand (static_cast <unsigned> (time(0)));
    // int delay_inc = NUM_LOOPS / 5;
    // int delay = 80000; //delay_inc;
    int elapsed_us = 0;
    
    for(size_t i=0; i < NUM_LOOPS; i++) {
        // float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float r = rand() / (float)RAND_MAX;

        auto start = high_resolution_clock::now();
        buf.push(r);
        float f1 = buf.read(500);
        float f2 = buf.read(5000);
        float f3 = buf.read(50000);
        buf.add(50010, f3);
        buf.mix(70000, f2);
        float f4 = buf.read(4000);
        buf.write(90000, f1);
        buf.add(4025, f4);
        auto stop = high_resolution_clock::now();


        auto duration = duration_cast<microseconds>(stop - start);
        elapsed_us += duration.count();
    }

    std::cout << "Total time: " << elapsed_us << "us" << std::endl;

 
    return 0;
}
