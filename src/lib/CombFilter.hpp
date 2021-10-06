/*
 * A comb filter with variable g and delay.
 */
#ifndef COMB_FILTER
#define COMB_FILTER
#include "DelayBuffer.hpp"

// template<size_t MAX_DELAY>
class CombFilter {
    DelayBuffer<float>  _x;
    DelayBuffer<float>  _y;
    float _g;
    int _delay;

    CombFilter();

public:

    CombFilter(size_t max_size, float g=0.5f, int delay=0) :
        _x(max_size),
        _y(max_size),
        _g(g),
        _delay(delay > 0 ? delay : max_size-1)
    {}

    void g(float g) {
        _g = g;
    }

    void delay(int d) {
        _delay = d;
    }

    int delay() {
        return _delay;
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