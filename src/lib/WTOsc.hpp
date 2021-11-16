#ifndef WAVE_TABLE2_HPP
#define WAVE_TABLE2_HPP

#include "Wavetable.hpp"

// #include <mutex>
// #include <vector>



class WTIOsc {
    WaveTablePtr _wavetable;
    size_t _idx = 0;

public:
    WTIOsc(WaveTablePtr wavetable, size_t start=0) :
        _wavetable(wavetable),
        _idx(start) {}

    inline float next() {
        float out = _wavetable->at(_idx);
        if(++_idx >= _wavetable->size()) {
            _idx = 0;;
        }
        return out;
    }
};

class WTFOsc {
    WaveTablePtr _wavetable;
    float _idx;
    float _inc = 1.0;

public:
    WTFOsc(WaveTablePtr wavetable, float increment, float start=0.f) :
        _wavetable(wavetable),
        _idx(start),
        _inc(increment) {}

    inline float next(float nudgeInc=0.f) {
        float out = _wavetable->atF(_idx);
        _idx += _inc;
        size_t len = _wavetable->size();
        if(_idx >= len) {
            _idx -= len;
        }
        _inc += nudgeInc;
        return out;
    }

    inline void inc(float i) {
        _inc = i;
    }
};

#endif
