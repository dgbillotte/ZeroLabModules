#ifndef UTIL_HPP
#define UTIL_HPP

#include <chrono>
using namespace std::chrono;

/*-----------------------------------------------------------------------------
 These functions provide audio like tapers for x:[0..1], y[0..1]
*/
// fast near 0, slow to 1
float audioTaperExp(float x);
float audioTaperX2(float x);
float audioTaperX3(float x);
float audioTaperX4(float x);

// slow near 0, fast to 1
float audioTaperLog(float x);
float audioTaperX2Inv(float x);
float audioTaperX3Inv(float x);
float audioTaperX4Inv(float x);

// this is x[0,2] where [0,1] is log and [1,2] is exp
// this makes is slow at the ends and fast in the middle
float audioTaperSlowEnds2(float x);
float audioTaperSlowEnds3(float x);
float audioTaperSlowEnds4(float x);

template <typename T>
struct Stats {
    T min = 10.f;
    T max = -10.f;
    T sum = 0.f;
    int count = 0;

    void sample(T sample) {
        sum += sample;
        if(sample > max)
            max = sample;
        if(sample < min)
            min = sample;
    }
    void reset() {
        min = 10.f;
        max = -10.f;
        sum = 0.f;
        count = 0;
    }

    T mean() { return sum/(T)((count > 0) ? count : 0.00001f); }
};
typedef Stats<float> StatsF;


/*
 * BlockTimer
 * what: light-weight timer container for timing blocks of code
 */
class BlockTimer {
    high_resolution_clock::time_point _start;
    high_resolution_clock::time_point _end;
    int _totalTime = 0;
    int _lapTime = 0;
    int _numLaps = 0;


public:
    void start() {
        _start = high_resolution_clock::now();
    }

    // void pause() {
    //     _end = high_resolution_clock::now();
    //     auto duration = duration_cast<microseconds>(_end - _start);
    //     _lapTime += duration.count();
    // }

    // void unpause() {
    //     _start = high_resolution_clock::now();
    // }

    void lap() {
        _end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(_end - _start);
        // auto duration = duration_cast<milliseconds>(_end - _start);
        _totalTime += _lapTime + duration.count();
        _lapTime = 0;
        _numLaps++;
    }

    int numLaps() {
        return _numLaps;
    }

    float aveLap() {
        return (_numLaps > 0) ? (((float)_totalTime) / (float)_numLaps) : 0.f;
    }

    void reset() {
        _totalTime = _lapTime = _numLaps = 0;
    }
    
};


#endif