#ifndef KARPLUS_STRONG_HPP
#define KARPLUS_STRONG_HPP

#include "DelayBuffer3.hpp"
#include "Filter.hpp"
class KarplusStrong {
    int _sampleRate;
    int _delayLength = 1;
    DelayBuffer3<float> _delayLine;
    DelayBuffer3<float> _impulseDelay;
    float _Pc = 0.99f; // calculated value for the phase coefficient for tuning correction
    float _p = 1.f; // decay factor in 0..1
    float _S = 0.5f; // decay "stretching" factor in 0..1. 0.5 produces most minimal decay
    float _dynamicLevel = 10.f;  // dynamics gain level
    float _R = 0.f; // calculated value for R for the dynamics filter
    int _write_i = 1000000;
    int _attack_on = 0;
    float _pickPos = 0.f;

    TwoPoleBPF _impulseBPF;


public:
    KarplusStrong(int sampleRate, int maxDelay=5000) :
        _sampleRate(sampleRate),
        _delayLine(maxDelay),
        _impulseDelay(maxDelay),
        _impulseBPF(sampleRate, 440.f, 0.01f)
    {
        _delayLine.clear();
        _impulseDelay.clear();
    }

    ~KarplusStrong() {}
    void sampleRate(int sampleRate) { _sampleRate = sampleRate; }
    // Decay needs to be |p| <= 1. todo: try out neg values
    void p(float p) { _p = p; }
    // Stretch needs to be 0 < S < 1
    void S(float S) { _S = S; }
    void dynamicsLevel(float level) { _dynamicLevel = level; } //This should be greater than 1
    // pickPos should be 0 <= pickPos <= 1
    void pickPos(float pos) { _pickPos = pos; }

    // for now, attackLength is a multiplier, not actual length.
    // In should be in [0,5+], neg values not good
    void pluck(float freq, float attackLength=1.f) {
        // calculate the _delayLength and _Pc parameter
        _setFreqParams(freq);
        _setImpulseParams(freq, attackLength);
        // _computeR(freq);

        // set attack length and initiate the writing of the impulse
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
        float out = (x + _p*((1-_S)*y0 + _S*y1))/2; // the 2nd /2 isn't mentioned, but it blows up without it....
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
    float _impulseY1 = 0.f;
    void _excite() {
        if(_write_i < _delayLength) {
            // initial sample value
            float impulseY0 = ((rand() / (float)RAND_MAX) *2.f - 1.f);

            // dynamics filter
            // if(_R) {
            //     impulseY0 = (1 - _R) * impulseY0 + _R * _impulseY1;
            // }
            //
            impulseY0 = _impulseBPF.process(impulseY0);

            // pick position filter
            impulseY0 -= _impulseDelay.read();

            _delayLine.push(impulseY0);
            _write_i++;
        }

        if(_attack_on > 0) {
            _attack_on--; // this should NOT go negative
        }
    }

    /*
     * This isn't working as of now. It has made some noise, but then cuts out.
     * I'm not sure about some of the places where I've used _freq and f1 below.
     * In the text "f" is used only once and "f1" is used in an ambiguous way and
     * I'm not sure if the lines for tmp and pift are correct
     * 
     * todo:
     * - analyze the values coming out to see where it is falling off
     * - create simpler approximation of this value. currently it has:
     *   - 7 expensive func calls: exp, sqrt, cos, sin
     *   - 21 mulitiply/divides
     */
    // void _computeR(float freq) {
    //     float f1 = 20.f;
    //     float fu = 10000.f;
    //     float fm = sqrt(f1*fu);
    //     float Rl = exp(-M_PI * _dynamicLevel / _sampleRate);
    //     float tmp = (-freq * 2.f * M_PI * fm) / _sampleRate;
    //     float Gl = (1.f - Rl) / (1.f - Rl*exp(tmp));
    //     float G2 = Gl*Gl;
    //     float pift = M_PI * f1 / _sampleRate;
    //     float p1 = (1.f - G2 * cos(2.f * pift)) / (1.f - G2);
    //     float cospift = cos(pift);
    //     float p2 = (2.f * Gl * sin(pift) * sqrt(1.f - (G2 * cospift * cospift))) / (1.f - G2);
    //     float r1 = p1 + p2;
    //     _R = (r1 < 1) ? r1 : (p1 - p2);
    // }
    
    // do all the nasty calculations needed on a freq change
    void _setFreqParams(float freq) {
        // float Pa = 0.5f; // use this if not using stretch (_S)
        float wT = 2.f*M_PI*freq / _sampleRate;
        float Pa = -atan((-_S*sin(wT))/((1.f - _S) + _S * cos(wT))) / wT;
        float P1 = _sampleRate/freq;
        _delayLength = P1 - Pa - 0.00001f;
        _delayLine.size(_delayLength);
        _Pc = P1 - _delayLength - Pa;
    }

    void _setImpulseParams(float freq, float attackLength) {
        //_computeR(freq);
        _impulseDelay.size(_delayLength * _pickPos);
        _impulseBPF.freq(freq);
        _impulseBPF.q(_dynamicLevel);
        _attack_on = _delayLength * attackLength;
        _write_i = 0;
    }
};

#endif