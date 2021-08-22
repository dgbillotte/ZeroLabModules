#include "AudioLib.hpp"


//--------------------------------------------------------
class OscBase {
public:
    float _phase = 0.f;
    
protected:
    float _freq = 0.f;
    float _phaseInc = 0.f;
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


typedef std::function<float(float)> F2F;

class DIYQuadrant : public SingleOsc {
public:
    void setQ1(F2F f) { _q1 = f; }
    void setQ2(F2F f) { _q2 = f; }
    void setQ3(F2F f) { _q3 = f; }
    void setQ4(F2F f) { _q4 = f; }
    
    void setAllQ(F2F f1,
                 F2F f2,
                 F2F f3,
                 F2F f4) {
        _q1 = f1;
        _q2 = f2;
        _q3 = f3;
        _q4 = f4;
    }


protected:
    float _getValue() override {
        float out = 0.f;
        if(_phase < _PI/2.f)
            out = _q1(_phase);
        else if(_phase < _PI)
            out = _q2(_phase);
        else if(_phase < _3halfPI)
            out = _q3(_phase);
        else
            out = _q4(_phase);
        return clamp(out, -1.f, 1.f);
    }
    
    F2F _q1;
    F2F _q2;
    F2F _q3;
    F2F _q4;
};

//--------------------------------------------------------
template<class T>
class MultipleVoiceOsc : public OscBase {
public:
    float getValue(T type) { return _getValue(type); }
    int numVoices() { return T::NUM_VOICES; }
    void next() { _cycle(); }
    
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



