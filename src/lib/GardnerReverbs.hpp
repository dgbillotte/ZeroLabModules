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
    float _g = 0.f;
    int _sampleRate;
    DelayBuffer<float> _delayLine;
    SimpleOnePoleLPF _lpf;
    float _preDelay = 0;
    float _longDelay = 0;
    int _dirty = false;
    
public:
    GardnerReverb(int sample_rate, int delayLength, int lpfFreq, float g=0.5f) :
        _g(g),
        _sampleRate(sample_rate),
        _delayLine(_mstos(delayLength)),
        _lpf(lpfFreq, sample_rate)
    {}

    // setters for the common parameters
    void g(float g) { _g = g; }
    void lpfFreq(float freq) { _lpf.freq(freq); }
    void sampleRate(int sample_rate) { _sampleRate = sample_rate; }
    void preDelay(float preDelay) {
        if(_preDelay == preDelay)
            return;
        
        if(preDelay > 1.f)
            _preDelay = 1.f;
        else if(preDelay < -1.f)
            _preDelay = -1.f;
        else
            _preDelay = preDelay;
        _dirty = true;

    }

    void longDelay(float longDelay) {
        if(_longDelay == longDelay)
            return;

        if(longDelay > 1.f)
            _longDelay = 1.f;
        else if(longDelay < -1.f)
            _longDelay = -1.f;
        else
            _longDelay = longDelay;
        _dirty = true;
    }

    float process(float input) {
        if(_dirty) {
            _updateFilters();
            _dirty = false;
        }
        return _process(input);
    }

protected:
    // override this to implement a specific topology
    virtual float _process(float input) = 0; 
    
    virtual void _updateFilters() {}

    // Helper method to convert millisecs -> samples
    int _mstos(float ms) {
        return ms * _sampleRate/1000;
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
        // group 1
        _apf1(&_delayLine, _mstos(24), _mstos(35), 0.3f),
        _apf2(&_delayLine, _mstos(25), _mstos(22), 0.4f),
        _apf3(&_delayLine, _mstos(48), _mstos(8.3), 0.6f),

        // group 2
        _apf4(&_delayLine, _mstos(60), _mstos(66), 0.1f),
        _apf5(&_delayLine, _mstos(61), _mstos(30), 0.4f)
    {}

    void _updateFilters() override {
        _apf1.delay(_mstos(35.f+(0*_preDelay)));
        _apf2.delay(_mstos(22.f+(11.f*_preDelay)));
        _apf3.delay(_mstos(8.3f+(5.f*_preDelay)));
        // _apf4.delay(_mstos(66.f+(47.f*_longDelay)));
        _apf5.delay(_mstos(30.f+(30.f*_longDelay)));
    }
    
    float _process(float input) override {
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
        _apf1(&_delayLine, _mstos(0), _mstos(35), 0.3f),
        _apf2(&_delayLine, _mstos(0), _mstos(8.3), 0.7f),
        _apf3(&_delayLine, _mstos(9), _mstos(22), 0.5f),
        _apf4(&_delayLine, _mstos(40), _mstos(30), 0.5f),
        _apf5(&_delayLine, _mstos(152), _mstos(39), 0.3f),
        _apf6(&_delayLine, _mstos(152), _mstos(9.8), 0.6f)
    {}

    float _process(float input) override {
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
        _apf1(&_delayLine, _mstos(0), _mstos(8), 0.3f),
        _apf2(&_delayLine, _mstos(8), _mstos(12), 0.3f),
        _apf3(&_delayLine, _mstos(41), _mstos(87), 0.5f),
        _apf4(&_delayLine, _mstos(41), _mstos(62), 0.25f),
        _apf5(&_delayLine, _mstos(152), _mstos(120), 0.5f),
        _apf6(&_delayLine, _mstos(152), _mstos(76), 0.25f),
        _apf7(&_delayLine, _mstos(228), _mstos(30), 0.25f)
    {}

    float _process(float input) override {
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