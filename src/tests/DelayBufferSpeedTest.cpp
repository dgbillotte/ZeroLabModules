#include <iostream>
#include "DelayBuffer.hpp"
// #include <cstdlib>
//#include <ctime>

#include <chrono>
using namespace std::chrono;

void pushN(DelayBuffer<float> &buf, int n) {
    for(int i=0; i<n; i++)
        buf.push(rand() / (float)RAND_MAX);
}

int main() {

    const size_t MAX_DELAY = 10000;
    const size_t NUM_LOOPS = 10;
    // Basic Circular Buffer Tests

    DelayBuffer<float> buf(MAX_DELAY);
    // buf.dump();

    srand (static_cast <unsigned> (time(0)));
    // int delay_inc = NUM_LOOPS / 5;
    // int delay = 80000; //delay_inc;
    int elapsed_us = 0;
    
    pushN(buf, MAX_DELAY/2);


    for(size_t i=0; i < NUM_LOOPS; i++) {
        // float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        // float r = rand() / (float)RAND_MAX;

        auto start = high_resolution_clock::now();
        buf.size(MAX_DELAY*0.75);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        elapsed_us += duration.count();

        pushN(buf, MAX_DELAY*0.1);
        
        start = high_resolution_clock::now();
        buf.size(MAX_DELAY*0.25);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        elapsed_us += duration.count();

        pushN(buf, MAX_DELAY*0.1);

        start = high_resolution_clock::now();
        buf.size(MAX_DELAY*0.6);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        elapsed_us += duration.count();
        
        pushN(buf, MAX_DELAY*0.1);

        start = high_resolution_clock::now();
        buf.size(MAX_DELAY*0.7);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        elapsed_us += duration.count();
        
        pushN(buf, MAX_DELAY*0.1);

        start = high_resolution_clock::now();
        buf.size(MAX_DELAY*0.8);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        elapsed_us += duration.count();

    }

    std::cout << "Total Time: " << elapsed_us << "us" << std::endl;
    std::cout << "Average time / Resize: " << elapsed_us/(5*NUM_LOOPS) << "us" << std::endl;

 
    return 0;
}
