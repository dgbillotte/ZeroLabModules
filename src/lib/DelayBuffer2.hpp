/*
 * This is a very simple circular buffer meant specifically
 * as a delay, hence the simple interface. It has no notion of
 * how full it is and provides no protections against invalid 
 * addressing. 
 */

/*
 When writing this and testing it I had to continually remind
 myself of the invariant:
 - except during push() and clear(), _head *always* points to the D[0]
 */

#ifndef DELAY_BUFFER2
#define DELAY_BUFFER2
#include <cstring>

template<typename T>
class DelayBuffer2 {

    T* _buf;
    const size_t MAX_SIZE;
    T* _headp;

    DelayBuffer2() { ; }

public:
    DelayBuffer2(size_t max_size) :
        _buf(new T[max_size]),
        MAX_SIZE(max_size),
        _headp(_buf + MAX_SIZE)
    { }

    void push(T t) {
        // increment head, wrap if necessary
        if(++_headp >= _buf + MAX_SIZE)
            _headp = _buf;

        *_headp = t;
    }

    T read(int delay=-1) {
        if(delay == -1)
            delay = MAX_SIZE;

        return *(mask(delay));
    }

    // overwrite a delay-line entry
    void write(time_t delay, T t) {
        *(mask(delay)) = t;
    }

    // add to a delay-line entry
    void add(time_t delay, T t) {
        *(mask(delay)) += t;
    }
    
    // mix new value into a delay-line entry
    void mix(time_t delay, T t, float mix=0.5f) {
        T* idx = mask(delay);
        *(idx) += (mix * t) + ((1-mix) * *(idx));    
    }

    void clear() {
        memset(_buf, 0, MAX_SIZE*sizeof(T));
        _headp = _buf+MAX_SIZE; // this isn't *necessary* but does make it "just like brand new"
    }

    T* mask(size_t t) {
        T* p = _headp - t;
        if(p < _buf)
            p += MAX_SIZE;
        return p;
    }

    // purely for debugging
    void dump() {
        std::cout << "CB2 Dump" << std::endl;
        std::cout << "\tbuffer size: " << MAX_SIZE << std::endl <<
            "\thead: " << _headp << std::endl << "\telements: [";
        for(size_t i=0; i<MAX_SIZE; i++) {
            std::cout << _buf[i] << ",";
        }
        std::cout << "]" << std::endl;
    }    

};


#endif