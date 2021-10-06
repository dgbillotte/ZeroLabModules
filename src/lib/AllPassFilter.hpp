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

    void g(float g) { _g = g; }
    void delay(int d) { _delay = d; }
    int delay() { return _delay; }

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
    size_t _end;
    float _g;

    NestedAllPassFilter();

public:
    NestedAllPassFilter(DelayBuffer<float>* buffer, size_t start, size_t delay, float g) :
        _buf(buffer), _start(start), _end(start+delay), _g(g)
    {}

    void start(size_t start) { _start = start; }
    void delay(size_t delay) { _end = _start + delay; }
    void g(float g) { _g = g; }


    // this method is still psychotic. The first version passes x0
    // in as a param, this matches other process signatures. I like this,
    // but the second form (reads x0 from the delayline) has nice
    // symmetrical form and might be nice from pipelining, we'll see.
    //
    // Originally, the x0 param was just to fit-in and made no difference as
    // x0 was read from the buffer to follow the algorithm. Now I have
    // a diff idea. If there will be a number of these strung together,
    // it could make sense for the first one to take the input and all
    // of the subsequent ones to take the parameterless
    float process(float x0) {
        _buf->push(x0);
        float xD = _buf->read(_end);
        float y0 = -_g * x0 + xD;
        x0 += _g * y0;

        _buf->write(_end, y0);
        _buf->write(_start, x0);

        return y0;
    }

    float process() {
        float x0 = _buf->read(_start);
        float xD = _buf->read(_end);
        float y0 = -_g * x0 + xD;
        x0 += _g * y0;

        _buf->write(_end, y0);
        _buf->write(_start, x0);

        return y0;
    }

    void dump() {
        std::cout << "NestedAllPassFilter Dump" << std::endl;
        std::cout << "_start: " << _start << ", _end: " << _end <<
            ", g: " << _g << std::endl;
        _buf->dump();
    }
};

#endif
