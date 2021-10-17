#ifndef KARPLUS_STRONG_HPP
#define KARPLUS_STRONG_HPP

#include "DelayBuffer.hpp"
// #include "Filter.hpp"
// #include "AllPassFilter.hpp"




class KarplusStrong {
    int _sampleRate;
    int _delayLength = 1;
    DelayBuffer<float> _delayLine;
    float _Pc = 0.99f;
    float _p = 1.f; // in 0..1
    float _S = 0.5f; // in 0..1
    int _write_i = 1000000;
    int _attack_on = false;
    // SimpleOnePoleLPF _lpf;

public:
    KarplusStrong(int sampleRate, int maxDelay=5000) :
        _sampleRate(sampleRate),
        _delayLine(maxDelay)
        // _lpf(440, sampleRate)
    {}

    ~KarplusStrong() {}

    void sampleRate(int sampleRate) {
        _sampleRate = sampleRate;
        // _lpf.sampleRate(_sampleRate);
    }
    // void lpfFreq(float freq) {
    //     _lpf.freq(freq);
    // }

    void p(float p) {
        _p = p;
    }

    void S(float S) {
        _S = S;
    }

    void pluckOld(float freq, int useWavetable=false) {
        _delayLength = round(_sampleRate/freq + 0.5f);

        float Pa = 0.5f;

        float wT = 2*M_PI*freq / _sampleRate;

        Pa = -atan((-_S*sin(wT))/((1 - _S) + _S * cos(wT))) / wT;

        float P1 = _sampleRate/freq;
        _delayLength = P1 - Pa - 0.00001f;
        _Pc = P1 - _delayLength - Pa;
        // std::cout << "freq: " << freq << " delay: " << _delayLength <<
        //     " sr: " << _sampleRate << std::endl;
        _excite();

        // return nextValue();
    }

    // for now, attackLength is a meta param, not actual length. In should be in [0,5], neg values not good
    void pluck(float freq, float attackLength=1.f) {
        // calculate the _delayLength and _Pc parameter
        float Pa = 0.5f;
        float wT = 2*M_PI*freq / _sampleRate;
        Pa = -atan((-_S*sin(wT))/((1 - _S) + _S * cos(wT))) / wT;
        float P1 = _sampleRate/freq;
        _delayLength = P1 - Pa - 0.00001f;
        _Pc = P1 - _delayLength - Pa;

        // set attack length and initiate the writing of the impulse
        _attack_on = _delayLength * attackLength;
        _write_i = 0;
    }

    float nextValue(int log=false) {
        _excite2();

        const float gain = 10.f;
        float x = _delayLine.read(0);

        if(_attack_on) {
            return x * gain;
        }

        float y0 = _delayLine.read(_delayLength);
        float y1 = _delayLine.read(_delayLength+1);

        // this is the standard KP with a 2-point average
        // float out = (x + (y0 + y1)/2)/2; // the 2nd /2 isn't mentioned, but it blows up without it....

        // KP with a 2-point weighted average
        float out = (x + ((1-_S)*y0 + _S*y1))/2; // the 2nd /2 isn't mentioned, but it blows up without it....

        // out = _lpf.process(out);
        _delayLine.push(out);


        // all-pass filter to correct tuning
        float Pc = 0.5f;
        float C = (1 - Pc) / (1 + Pc);
        out = C * out + x - C * y0;

        // if(log) {
        //     std::cout << "x + (y0+y1)/2 : " << (x + (y0 + y1)/2) << std::endl;
        // }


        return out * gain;
    }

    float currentValue() {
        return _delayLine.read(0);
    }

protected:
    // I tested this and it is indeed creating values in -1..1 with a mean of 0
    void _excite() {
        for(int i=0; i < _delayLength; i++) {
            float s = ((rand() / (float)RAND_MAX) *2.f - 1.f);
            _delayLine.push(s);
        }
    }

    // keep writing the impulse util it is _delayLength long
    // countdown the _attack
    void _excite2() {
        if(_write_i < _delayLength) {
            float s = ((rand() / (float)RAND_MAX) *2.f - 1.f);
            _delayLine.push(s);
            _write_i++;
        }

        if(_attack_on > 0) {
            _attack_on--; // this should NOT go negative
        }
    }
};

#endif