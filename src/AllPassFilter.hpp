/*
 * An all-pass filter with variable g and delay.
 */
#ifndef ALL_PASS_FILTER
#define ALL_PASS_FILTER
#include "DelayBuffer.hpp"

// template<size_t MAX_DELAY>
class AllPassFilter {
    DelayBuffer<float>  _x;
    DelayBuffer<float>  _y;
    int _delay;
    float _g = 0.f;

    AllPassFilter();

public:

    AllPassFilter(size_t max_size) : _x(max_size), _y(max_size), _delay(max_size-1) {}

    void g(float g) {
        _g = g;
    }

    void delay(int d) {
        _delay = d;
    }

    float process(float x0) {
        float xD = _x.read(_delay);
        float yD = _y.read(_delay);
        float y0 = -(_g * x0) + xD + (_g * yD);

        _x.push(x0);
        _y.push(y0);

        return y0;
    }

};

#endif
