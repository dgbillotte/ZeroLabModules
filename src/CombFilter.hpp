/*
 * A comb filter with variable g and delay.
 */
#ifndef COMB_FILTER
#define COMB_FILTER
#include "DelayBuffer.hpp"

template<size_t MAX_DELAY>
class CombFilter {
    DelayBuffer<float, MAX_DELAY>  _x;
    DelayBuffer<float, MAX_DELAY>  _y;
    float _g = 0.f;
    int _delay = MAX_DELAY-1;

public:
    void g(float g) {
        _g = g;
    }

    void delay(int d) {
        _delay = d;
    }

    float process(float x0) {
        float xD = _x.read(_delay);
        float yD = _y.read(_delay);
        float y0 = xD + (_g * yD);

        _x.push(x0);
        _y.push(y0);

        return y0;        
    }
    
};

#endif