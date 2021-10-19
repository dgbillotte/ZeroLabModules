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

#ifndef DELAY_BUFFER
#define DELAY_BUFFER
#include <cstring>

template<typename T>
class DelayBuffer {

    T* _buf;
    T* _tmp;
    size_t _head;
    size_t _end;
    const size_t MAX_SIZE;

    DelayBuffer() { ; }

public:
    DelayBuffer(size_t max_size, size_t size=0) :
        _buf(new T[max_size]),
        _tmp(new T[max_size]),
        _head(size-1),
        _end((size > 0) ? size : max_size),
        MAX_SIZE(max_size)
    {}

    size_t size() { return _end; }

    void size(size_t newSize) {
        if(newSize > MAX_SIZE)
            throw std::length_error("newSize is greater than MAX_SIZE");

        int delta = newSize - _end;
        if(delta == 0)
            return;

        T *dst1, *src1, *dst2, *src2;
        size_t sz1, sz2;

        int pivot = _head+1;
        size_t sizeT = sizeof(T);

        int tailLen = (_end-_head)-1;

        if(delta > 0) { // growing
            // move tail of list to front
            dst1 = _tmp;
            src1 = &(_buf[pivot]);
            sz1 = tailLen * sizeT;
            memcpy(dst1, src1, sz1);

            // move head of list to rear
            dst2 = &(_tmp[tailLen]);
            src2 = _buf;
            sz2 = pivot * sizeT;
            memcpy(dst2, src2, sz2);
            _head = _end-1;
            _end = newSize;

        } else { // shrinking
            // make delta positive
            delta = -delta;

            if(delta >= tailLen) { // we don't need the tail
                int takeFromHead = delta - tailLen;

                dst1 = _tmp;
                src1 = &(_buf[takeFromHead]);
                sz1 = ((_head - takeFromHead)+1) * sizeT;
                memcpy(dst1, src1, sz1);
                _end = newSize;
                _head = _end-1;

            } else {
                int takeFromTail = tailLen - delta;

                dst1 = _tmp;
                src1 = &(_buf[_end - takeFromTail]);
                sz1 = takeFromTail * sizeT;
                memcpy(dst1, src1, sz1);

                dst2 = &(_tmp[takeFromTail]);
                src2 = _buf;
                sz2 = pivot * sizeT;
                memcpy(dst2, src2, sz2);
                _end = newSize;
                _head = _end - 1;
            }
        } 

        // copy tmp back to _buf
        // TODO: use a ** to just swap between the two to eliminate this copy
        memcpy(_buf, _tmp, newSize*sizeT);
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

    // overwrite a delay-line entry
    void write(time_t delay, T t) {
        _buf[dtoi(delay)] = t;
    }

    // add to a delay-line entry
    void add(time_t delay, T t) {
        _buf[dtoi(delay)] += t;
    }
    
    // mix new value into a delay-line entry
    void mix(time_t delay, T t, float mix=0.5f) {
        int idx = dtoi(delay);
        _buf[idx] = (mix * t) + ((1-mix) * _buf[idx]);    
    }

    void clear(T* buf=NULL) {
        if(buf == NULL) {
            buf = _buf;
        }
        
        memset(buf, 0, MAX_SIZE*sizeof(T));
        _head = _end-1; // this isn't *necessary* but does make it "just like brand new"
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