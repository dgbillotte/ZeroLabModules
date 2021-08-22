#include "AudioLib.hpp"


//--------------------------------------------------------
class OscBase {
protected:
    float _freq = 0.f;
    float _phaseInc = 0.f;
    float _phase = 0.f;
    int _sampleRate = 0;
    
public:
    void setFreq(float freq) {
        if(_freq != freq) {
            _freq = freq;
            _calcPhaseInc();
        }
    }
    
    void setSampleRate(int sampleRate) {
        if(_sampleRate != sampleRate) {
            _sampleRate = sampleRate;
            _calcPhaseInc();
        }
    }

    void setPhaseParams(float freq, int sampleRate) {
        _freq = freq;
        _sampleRate = sampleRate;
        _calcPhaseInc();
    }
    
protected:
    void _calcPhaseInc() {
        _phaseInc = _twoPI * _freq / _sampleRate;
        _initCycle();
    }
    
    void _cycle() {
        _phase += _phaseInc;
        if(_phase > _twoPI)
            _phase -= _twoPI;
    }
    
    virtual void _initCycle() { ; }
};


//--------------------------------------------------------
class SingleOsc : public OscBase {
public:
    float getValue() {
        float out = _getValue();
        _cycle();
        return out;
    }
protected:
    virtual float _getValue() = 0;
};

//--------------------------------------------------------
class SineOsc : public SingleOsc {
protected:
    float _getValue() override {
        return sin(_phase);
    }
};

//--------------------------------------------------------
template<class T>
class MultipleVoiceOsc : public OscBase {
public:
    
    float getValue(T type) {
        return _getValue(type);
    }

    int numVoices() {
        return T::NUM_VOICES;
    }
    
    void next() {
        _cycle();
    }
    
protected:
    virtual float _getValue(T type) = 0;
};


//--------------------------------------------------------
enum SineSawVoices {
    VOICE_SINE,
    VOICE_SAW,
    NUM_VOICES
};

class SineSawOsc : public MultipleVoiceOsc<SineSawVoices> {
protected:
    float _getValue(SineSawVoices type) override {
        if(type == VOICE_SINE)
            return _sin_out;
           
        // type == VOICE_SAW
        return _phase/_PI - 1.f;
    }
    
protected:
    float _sin_out=0;
    void _initCycle() override {
        _sin_out = sin(_phase);
    }
};



