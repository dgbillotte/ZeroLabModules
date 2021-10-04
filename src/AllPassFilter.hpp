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
    float _g;
    int _delay;

    AllPassFilter();

public:
    AllPassFilter(size_t max_size, float g=0.5f, int delay=0) :
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
        float y0 = -(_g * x0) + xD + (_g * yD);

        _x.push(x0);
        _y.push(y0);

        return y0;
    }
};


class NestedAllPassFilter {
    DelayBuffer<float>* _buf;
    size_t _start;
    size_t _delay;
    float _g;

    NestedAllPassFilter();

public:
    NestedAllPassFilter(DelayBuffer<float>* buffer, size_t start, size_t end, float g) :
        _buf(buffer), _start(start), _delay(end), _g(g)
    {}

    void g(float g) {
        _g = g;
    }

    float process(float x0=0) {
        x0 = _buf->read(_start);
        float xD = _buf->read(_delay);
        float y0 = -_g * x0 + xD;
        x0 += _g * y0;

        _buf->write(_delay, y0);
        _buf->write(_start, x0);

        return y0;
    }
};

#endif
