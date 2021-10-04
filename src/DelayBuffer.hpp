/*
 * This is a very simple circular buffer meant specifically
 * as a delay, hence the simple interface. It has no notion of
 * how full it is and provides no protections against invalid 
 * addressing. 
 */

#ifndef DELAY_BUFFER
#define DELAY_BUFFER

template<typename T>
class DelayBuffer {
    T* _buf;
    size_t _head;// = MAX_SIZE-1;
    const size_t MAX_SIZE;

    DelayBuffer() { ; }

public:
    DelayBuffer(size_t max_size) :
        _buf(new T[max_size]),
        _head(max_size-1),
        MAX_SIZE(max_size) {}

    void push(T t) {
        // increment head, wrap if necessary
        if(++_head >= MAX_SIZE)
            _head = 0;

        _buf[_head] = t;
    }

    T read(int delay=-1) {
        if(delay == -1)
            delay = MAX_SIZE;
        // calc read point, wrap if necessary
        int i = _head - delay;
        if(i < 0)
            i+= MAX_SIZE;

        return _buf[i];
    }

    void clear() {
        memset(_buf, 0, MAX_SIZE*sizeof(T));
        _head = MAX_SIZE-1; // this isn't *necessary* but does make it "just like brand new"
    }

    // purely for debugging
    void dump() {
        std::cout << "CB2 Dump" << std::endl;
        std::cout << "\tbuffer size: " << MAX_SIZE << std::endl <<
            "\thead: " << _head << std::endl << "\telements: [";
        for(size_t i=0; i<MAX_SIZE; i++) {
            std::cout << _buf[i] << ",";
        }
        std::cout << "]" << std::endl;
    }
};


//---------------------------------------------------------------

template<typename T, size_t MAX_SIZE>
class DelayBufferOld {
    T _buf[MAX_SIZE];
    size_t _head = MAX_SIZE-1;

public:

    void push(T t) {
        // increment head, wrap if necessary
        if(++_head >= MAX_SIZE)
            _head = 0;

        _buf[_head] = t;
    }

    T read(size_t delay=MAX_SIZE) {
        // calc read point, wrap if necessary
        int i = _head - delay;
        if(i < 0)
            i+= MAX_SIZE;

        return _buf[i];
    }

    void clear() {
        memset(&_buf, 0, MAX_SIZE*sizeof(T));
        _head = MAX_SIZE-1; // this isn't *necessary* but does make it "just like brand new"
    }

    // purely for debugging
    void dump() {
        std::cout << "CB2 Dump" << std::endl;
        std::cout << "\tbuffer size: " << MAX_SIZE << std::endl <<
            "\thead: " << _head << std::endl << "\telements: [";
        for(size_t i=0; i<MAX_SIZE; i++) {
            std::cout << _buf[i] << ",";
        }
        std::cout << "]" << std::endl;
    }
};

#endif