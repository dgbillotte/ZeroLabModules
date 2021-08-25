#include <iostream>

using namespace std;
using namespace std::chrono;

/*
 *--------------------------------------------------------------
 * This is a benchmark of std::max(float, float) vs. my own
 * hand-rolled version. In my tests, my version is *slightly*
 * faster than std, on average... I wondered if the inlining
 * of the function was the key difference, so I added another
 * test with it explicitly NOT-inlined. After more testing
 * and pulling the data into pandas, my results are:
 * - my function (not inlined) is about 2% faster than std::max()
 * - my function (inlined) is about 8.6% faster than std::max()
 *
 * In conclusion, unless you are scrapping for every last
 * clock cycle, just use the built in function
 *
 * For the tests, this was built on osx like this:
 * > g++ test.cpp -std=c++11 -o test
 *--------------------------------------------------------------
 */


// below is an arbitrary "large" number, in this case:
// the number of samples in 1 min of audio
const int loops = 41000 * 60;

inline float my_max_inline(float a, float b) { return a > b ? a : b; }

[[gnu::noinline]]
float my_max(float a, float b) { return a > b ? a : b; }


int main() {
    int time_std=0, time_mymax=0, time_mymax_i=0;
    
    float accum=0;
    
    srand(time(NULL));
    float a, b, result;
    high_resolution_clock::time_point start, stop;
    
    for(int i=0; i < loops; i++) {
        a = 10 * rand() / RAND_MAX;
        b = 10 * rand() / RAND_MAX;
        
        auto start = high_resolution_clock::now();
        result = std::max(a,b);
        auto stop = high_resolution_clock::now();
        time_std += duration_cast<microseconds>(stop - start).count();
        
        // just to make sure the compiler does optimize away our test code
        accum += result;
    }
    
    for(int i=0; i < loops; i++) {
        a = 10 * rand() / RAND_MAX;
        b = 10 * rand() / RAND_MAX;
        
        auto start = high_resolution_clock::now();
        result = my_max(a,b);
        auto stop = high_resolution_clock::now();
        time_mymax += duration_cast<microseconds>(stop - start).count();

        // just to make sure the compiler does optimize away our test code
        accum += result;
    }

    for(int i=0; i < loops; i++) {
        a = 10 * rand() / RAND_MAX;
        b = 10 * rand() / RAND_MAX;
        
        auto start = high_resolution_clock::now();
        result = my_max_inline(a,b);
        auto stop = high_resolution_clock::now();
        time_mymax_i += duration_cast<microseconds>(stop - start).count();

        // just to make sure the compiler does optimize away our test code
        accum += result;
    }
    
    cout << time_std << "," << time_mymax << "," << time_mymax_i
        << "," << (((double)time_mymax)/time_std)
        << "," << (((double)time_mymax_i)/time_std) << endl;
    return 0;
}
