/*
 * This is a very simple circular buffer meant specifically
 * as a delay, hence the simple interface. It has no notion of
 * how full it is and provides no protections against invalid 
 * addressing. 
 */

/*
 When writing this and testing it I had to continually remind
 myself of the invariant:
 - except during push(), size(size_t) and clear(), _head *always* points to the D[0]
 */

#ifndef DELAY_BUFFER_HPP
#define DELAY_BUFFER_HPP
#include <cstring>

template<typename T>
class DelayBuffer {

    T* _buf;
    size_t _head;
    size_t _end;
    const size_t MAX_SIZE;

    DelayBuffer() { ; }

public:
    DelayBuffer(size_t max_size, size_t size=0) :
        _buf(new T[max_size]),
        _head(size-1),
        _end((size > 0) ? size : max_size),
        MAX_SIZE(max_size)
    {}

    size_t size() { return _end; }
    void size(size_t newSize) {
        if(newSize > MAX_SIZE) {
            std::string msg = std::string("newSize(") + std::to_string(newSize) + ") is greater than MAX_SIZE(";
            throw std::length_error(msg);
        }

        _end = newSize;
        if(_head >= _end)
            _head = 0;
    }

    // push a value into the delay-
    void push(T t) {
        // increment head, wrap if necessary
        if(++_head >= _end)
            _head = 0;

        _buf[_head] = t;
    }

    // read a delay-line entry
    T read(int delay=-1) {
        return _buf[dtoi(delay)];
    }

    T aRead(size_t pos) {
        return _buf[pos];
    }

    // overwrite a delay-line entry
    void write(int delay, T t) {
        _buf[dtoi(delay)] = t;
    }

    void aWrite(size_t pos, T t) {
        _buf[pos] = t;
    }

    // add to a delay-line entry
    void add(int delay, T t) {
        _buf[dtoi(delay)] += t;
    }

    void aAdd(size_t pos, T t) {
        _buf[pos] += t;
    }
    
    // mix new value into a delay-line entry
    void mix(int delay, T t, float mix=0.5f) {
        int idx = dtoi(delay);
        _buf[idx] = (mix * t) + ((1-mix) * _buf[idx]);    
    }

    void aMix(size_t pos, T t, float mix=0.5f) {
        _buf[pos] = (mix * t) + ((1-mix) * _buf[pos]);
    }

    void resetHead() {
        _head = _end-1;
    }

    void clear() {
        memset(_buf, 0, MAX_SIZE*sizeof(T));
        resetHead();
    }

    // I did this first with a loop and then with
    // memcpy as seen here. It is 10x faster going
    // from 8-15us to less than 1us
    float* getBuffer() {
        return _buf;
    }
    void fillBuffer(T* source, size_t len) {
        resetHead();
        if(len >= _end) {
            memcpy(_buf, source, _end * sizeof(float));
        } else {
            int i = 0, numCopies = _end / len;
            for(; i < numCopies; i++) {
                memcpy(&(_buf[i*len]), source, len * sizeof(float));
            }
            memcpy(&(_buf[i*len]), source, (_end - (i*len)) * sizeof(float));
        }
    }

    // purely for debugging
    void dump() {
        std::cout << "CB2 Dump" << std::endl;
        std::cout << "\tbuffer size: " << size() << std::endl <<
            "\thead: " << _head << std::endl << 
            "\tend: " << _end << std::endl <<
            "\telements: [";
        for(size_t i=0; i<MAX_SIZE; i++) {
            if(i == _head)
                std::cout << "^" << _buf[i] << ",";
            else 
                std::cout << _buf[i] << ",";

            if(i == _end-1)
                std::cout << "|";
                // std::cout << "^";
        }
        std::cout << "]" << std::endl;
    }

protected:
    // map a delay time to an index into the buffer 
    // positive delays are from the write head getting older as they get bigger
    // negative delays are from the end of the buffer and younger as they get more negative
    size_t dtoi(int delay) {
        if(delay < 0)
            delay = _end+delay;

        int i = _head - delay;
        if(i < 0)
            i+= _end;
        return i;
    }
};


#endif