#ifndef GARDNER_REVERB
#define GARDNER_REVERB
/*
 * Implementations of Gardner's reverbs as described by Pirkle
 */
#include "AllPassFilter.hpp"
#include "DelayBuffer.hpp"
#include "Filter.hpp"

class GardnerReverbSmall {
    float _g = 0;
    int _sample_rate;
    DelayBuffer<float> _delayLine;
    NestedAllPassFilter _apf1;
    NestedAllPassFilter _apf2;
    NestedAllPassFilter _apf3;
    NestedAllPassFilter _apf4;
    NestedAllPassFilter _apf5;
    SimpleOnePoleLPF _lpf;

public:
    GardnerReverbSmall(int sample_rate) :
        _sample_rate(sample_rate),
        _delayLine(mstos(126)),
        _apf1(&_delayLine, mstos(24), mstos(35), 0.3f),
        _apf2(&_delayLine, mstos(25), mstos(22), 0.4f),
        _apf3(&_delayLine, mstos(48), mstos(8.3), 0.6f),
        _apf4(&_delayLine, mstos(60), mstos(66), 0.1f),
        _apf5(&_delayLine, mstos(61), mstos(30), 0.4f),
        _lpf(3000, sample_rate)
    {}
    
    void g(float g) { _g = g; }
    void lpfFreq(float freq) { _lpf.freq(freq); }


    float process(float input) {
        float in = input + (_g * _lpf.process(_delayLine.read()));
        _apf1.process(in);
        _apf2.process();
        _apf3.process();
        float tap60 = _delayLine.read(60);
        _apf4.process();
        float delayOut = _apf5.process();

        float output = (delayOut + tap60)/2.f;
        return output;
    }
protected:
    int mstos(float sec) {
        return sec * _sample_rate/1000;
    }
};

class GardnerReverbMed {
    float _g = 0;
    int _sample_rate;
    DelayBuffer<float> _delayLine;
    NestedAllPassFilter _apf1;
    NestedAllPassFilter _apf2;
    NestedAllPassFilter _apf3;
    NestedAllPassFilter _apf4;
    NestedAllPassFilter _apf5;
    NestedAllPassFilter _apf6;
    SimpleOnePoleLPF _lpf;

public:
    GardnerReverbMed(int sample_rate) :
        _sample_rate(sample_rate),
        _delayLine(mstos(300)),
        _apf1(&_delayLine, mstos(0), mstos(35), 0.3f),
        _apf2(&_delayLine, mstos(0), mstos(8.3), 0.7f),
        _apf3(&_delayLine, mstos(9), mstos(22), 0.5f),
        _apf4(&_delayLine, mstos(40), mstos(30), 0.5f),
        _apf5(&_delayLine, mstos(152), mstos(39), 0.3f),
        _apf6(&_delayLine, mstos(152), mstos(9.8), 0.6f),
        _lpf(2500, sample_rate)
    {}
    
    void g(float g) { _g = g; }
    void lpfFreq(float freq) { _lpf.freq(freq); }


    float process(float input) {
        float in = input + (_g * _lpf.process(_delayLine.read()));

        _apf1.process(in);
        _apf2.process();
        _apf3.process();
        float tap35 = _delayLine.read(35);
        _apf4.process();
        float tap137 = _delayLine.read(137);
        _delayLine.write(152, input+(_g*_delayLine.read(152)));
        _apf5.process();
        float delayOut = _apf6.process();
        float output = (delayOut + tap35 + tap137)/2.f;

        return output;
    }
protected:
    int mstos(float sec) {
        return sec * _sample_rate/1000;
    }
};


#endif