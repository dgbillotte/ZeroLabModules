#ifndef GARDNER_REVERB
#define GARDNER_REVERB
/*
 * Implementations of Gardner's reverbs as described by Pirkle
 */
#include "AllPassFilter.hpp"
#include "DelayBuffer.hpp"
#include "Filter.hpp"

/*
 * This is the base class for the three Gardner reverbs presented in Pirkle.
 * It doesn't do much other than get rid of boilerplate code.
 */
class GardnerReverb {
protected:
    float _g = 0;
    int _sampleRate;
    DelayBuffer<float> _delayLine;
    SimpleOnePoleLPF _lpf;
    
public:
    GardnerReverb(int sample_rate, int delayLength, int lpfFreq, float g=0.5f) :
        _g(g),
        _sampleRate(sample_rate),
        _delayLine(mstos(delayLength)),
        _lpf(lpfFreq, sample_rate)
    {}

    // setters for the common parameters
    void g(float g) { _g = g; }
    void lpfFreq(float freq) { _lpf.freq(freq); }
    void sampleRate(int sample_rate) { _sampleRate = sample_rate; }

    // override this to implement a specific topology
    virtual float process(float input) = 0; 
protected:

    // Helper method to convert millisecs -> samples
    int mstos(float sec) {
        return sec * _sampleRate/1000;
    }
};

/*
 * Gardner small-room reverb.
 */
class GardnerReverbSmall : public GardnerReverb {
    NestedAllPassFilter _apf1;
    NestedAllPassFilter _apf2;
    NestedAllPassFilter _apf3;
    NestedAllPassFilter _apf4;
    NestedAllPassFilter _apf5;

public:
    GardnerReverbSmall(int sample_rate, float g=0.5f) :
        GardnerReverb(sample_rate, 126, 3000, g),
        _apf1(&_delayLine, mstos(24), mstos(35), 0.3f),
        _apf2(&_delayLine, mstos(25), mstos(22), 0.4f),
        _apf3(&_delayLine, mstos(48), mstos(8.3), 0.6f),
        _apf4(&_delayLine, mstos(60), mstos(66), 0.1f),
        _apf5(&_delayLine, mstos(61), mstos(30), 0.4f)
    {}
    
    float process(float input) override {
        // mix the input with filtered sample from the end of the delayLine
        float in = input + (_g * _lpf.process(_delayLine.read()));

        // run the filters and capture any needed tap values
        _apf1.process(in);
        _apf2.process();
        _apf3.process();
        float tap60 = _delayLine.read(60);
        _apf4.process();
        float delayOut = _apf5.process();

        // compose the output
        float output = (delayOut + tap60)/2.f;
        return output;
    }
};

/*
 * Gardner medium-room reverb.
 */
class GardnerReverbMed : public GardnerReverb {
    NestedAllPassFilter _apf1;
    NestedAllPassFilter _apf2;
    NestedAllPassFilter _apf3;
    NestedAllPassFilter _apf4;
    NestedAllPassFilter _apf5;
    NestedAllPassFilter _apf6;

public:
    GardnerReverbMed(int sample_rate, float g=0.5f) :
        GardnerReverb(sample_rate, 300, 2500, g),
        _apf1(&_delayLine, mstos(0), mstos(35), 0.3f),
        _apf2(&_delayLine, mstos(0), mstos(8.3), 0.7f),
        _apf3(&_delayLine, mstos(9), mstos(22), 0.5f),
        _apf4(&_delayLine, mstos(40), mstos(30), 0.5f),
        _apf5(&_delayLine, mstos(152), mstos(39), 0.3f),
        _apf6(&_delayLine, mstos(152), mstos(9.8), 0.6f)
    {}

    float process(float input) override {
        // mix the input with filtered sample from the end of the delayLine
        float in = input + (_g * _lpf.process(_delayLine.read()));

        // run the filters and capture any needed tap values
        _apf1.process(in);
        _apf2.process();
        _apf3.process();
        float tap35 = _delayLine.read(35);
        _apf4.process();
        float tap137 = _delayLine.read(137);
        _delayLine.write(152, input+(_g*_delayLine.read(152)));
        _apf5.process();
        float delayOut = _apf6.process();

        // compose the output
        float output = (delayOut + tap35 + tap137)/2.f;
        return output;
    }
};


/*
 * Gardner large-room reverb.
 */
class GardnerReverbLarge : public GardnerReverb {
    NestedAllPassFilter _apf1;
    NestedAllPassFilter _apf2;
    NestedAllPassFilter _apf3;
    NestedAllPassFilter _apf4;
    NestedAllPassFilter _apf5;
    NestedAllPassFilter _apf6;
    NestedAllPassFilter _apf7;

public:
    GardnerReverbLarge(int sample_rate, float g=0.5f) :
        GardnerReverb(sample_rate, 272, 2600, g),
        _apf1(&_delayLine, mstos(0), mstos(8), 0.3f),
        _apf2(&_delayLine, mstos(8), mstos(12), 0.3f),
        _apf3(&_delayLine, mstos(41), mstos(87), 0.5f),
        _apf4(&_delayLine, mstos(41), mstos(62), 0.25f),
        _apf5(&_delayLine, mstos(152), mstos(120), 0.5f),
        _apf6(&_delayLine, mstos(152), mstos(76), 0.25f),
        _apf7(&_delayLine, mstos(228), mstos(30), 0.25f)
    {}

    float process(float input) override {
        // mix the input with filtered sample from the end of the delayLine
        float in = input + (_g * _lpf.process(_delayLine.read()));

        // run the filters and capture any needed tap values
        _apf1.process(in);
        _apf2.process();
        float tap24 = _delayLine.read(24);
        _apf3.process();
        _apf4.process();
        float tap149 = _delayLine.read(149);
        _apf5.process();
        _apf6.process();
        float delayOut = _apf7.process();

        // compose the output
        float output = (0.34f * tap24) + (0.14f * tap149) + (0.14f * delayOut);
        return output;
    }
};

#endif