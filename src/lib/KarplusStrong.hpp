#ifndef KARPLUS_STRONG_HPP
#define KARPLUS_STRONG_HPP

#include "DelayBuffer.hpp"

class KarplusStrong {
    int _sampleRate;
    int _delayLength = 1;
    DelayBuffer<float> _delayLine;
    float _Pc = 0.99f;
    float _p = 1.f; // in 0..1
    float _S = 0.5f; // in 0..1
    int _write_i = 1000000;
    int _attack_on = 0;


public:
    KarplusStrong(int sampleRate, int maxDelay=5000) :
        _sampleRate(sampleRate),
        _delayLine(maxDelay)
    {
        _delayLine.clear();
    }

    ~KarplusStrong() {}
    void sampleRate(int sampleRate) { _sampleRate = sampleRate; }
    void p(float p) { _p = p; }
    void S(float S) { _S = S; }

    // for now, attackLength is a multiplier, not actual length.
    // In should be in [0,5], neg values not good
    void pluck(float freq, float attackLength=1.f) {
        // calculate the _delayLength and _Pc parameter
        _setFreqParams(freq);

        // set attack length and initiate the writing of the impulse
        _attack_on = _delayLength * attackLength;
        _write_i = 0;
    }

    void refret(float freq) {
        _setFreqParams(freq);
    }

    
    const float gain = 10.f;
    float nextValue(int log=false) {
        // run the exciter
        _excite();

        // get the "input"
        float x = _delayLine.read();

        // if attack is on, just return the noise
        if(_attack_on) {
            if(_write_i == _delayLength) {
                _delayLine.push(x);
            }
            return x * gain;
        }

        //
        float y0 = _delayLine.read(-1);
        float y1 = _delayLine.read(-2);

        // this is the standard KP with a 2-point average
        // float out = (x + (y0 + y1)/2)/2; // the 2nd /2 isn't mentioned, but it blows up without it....

        // KP with a 2-point weighted average
        float out = (x + ((1-_S)*y0 + _S*y1))/2; // the 2nd /2 isn't mentioned, but it blows up without it....
        _delayLine.push(out);

        // all-pass filter to correct tuning
        float Pc = 0.5f;
        float C = (1 - Pc) / (1 + Pc);
        out = C * out + x - C * y0;

        return out * gain;
    }

    float currentValue() {
        return _delayLine.read(0);
    }

protected:
    // keep writing the impulse util it is _delayLength long
    // countdown the _attack
    void _excite() {
        if(_write_i < _delayLength) {
            float s = ((rand() / (float)RAND_MAX) *2.f - 1.f);
            _delayLine.push(s);
            _write_i++;
        }

        if(_attack_on > 0) {
            _attack_on--; // this should NOT go negative
        }
    }
    
    // do all the nasty calculations needed on a freq change
    void _setFreqParams(float freq) {
        // float Pa = 0.5f;
        float wT = 2*M_PI*freq / _sampleRate;
        float Pa = -atan((-_S*sin(wT))/((1 - _S) + _S * cos(wT))) / wT;
        float P1 = _sampleRate/freq;
        _delayLength = P1 - Pa - 0.00001f;
        _delayLine.size(_delayLength);
        _Pc = P1 - _delayLength - Pa;
    }
};

#endif