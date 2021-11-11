#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
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


class CosLUT {
    int _numEntries;
    std::vector<float> _table;

public:
    CosLUT(int numEntries) : _numEntries(numEntries) {
        float inc = 2.f * M_PI / numEntries;
        for(float phase = 0.f; phase < 2.f * M_PI; phase += inc) {
            _table.push_back(cos(phase));
        }
    }

    float at(int idx) {
        return _table[idx];
    }

    // this is not working correctly!!!
    float atInterp(float fIdx) {
        int x0 = (int)fIdx;
        int x1 = (x0 + 1 < _numEntries) ? x0 + 1 : 0;
        float y0 = _table[x0];
        float y1 = _table[x1];
        return x0 + ((y1 - y0) * (fIdx - x0));
    }

    int size() { return _numEntries; }
};

#endif
