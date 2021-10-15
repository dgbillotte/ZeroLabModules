#ifndef BASE_CLASSES_HPP
#define BASE_CLASSES_HPP

/*
To facilitate easy combining of components, there should be some
high level base classes so that UGs can be composed together 
in an unrestricted manner.

Classes:
- Generator: Oscillators, EGs, noise, etc. No Signal input, just signal-out
- Filter/Effect/Processor: part of signal chain. signal-in/signal-out
- Mixers/Combiners: 1-many signal-in / 1-many signal out

Questions:
- how does a delay component fit in?
  - task: boil down its interface from a signal perspective

*/

template<typename T=float>
class SignalGeneratorT {
public:
    // get current value and kick it to the next cycle
    T nextValue(int type=0) {
        T v = currentValue(type);
        next();
        return v;
    }

    // get current value, but don't advance it
    T currentValue(int type=0);

    // advance to the next cycle
    void next() = 0;
}

template<typename T=float>
class ChainLinkT {
public:
    T process(T input) = 0;
}


#endif
