#ifndef WAVE_TABLE2_HPP
#define WAVE_TABLE2_HPP

#include "WaveTable.hpp"

/*
 * This will be the root baseclass for all oscillator
 * like things.
 */ 
class BasicOsc {
public:
    // this is to make sure that subclasses' destructors,
    // if any, get called
    virtual ~BasicOsc() { ; }

    // emits the next sample and moves forward one step
    virtual float next() = 0;
    virtual size_t length() = 0;

    // I'm not sure about this one, but it works right now
    virtual void restart() = 0;
};

typedef std::shared_ptr<BasicOsc> BasicOscPtr;


/*
 * An adapter that allows any signal to be transformed
 * into the output of an oscillator
 */
class ThruOsc;
typedef std::shared_ptr<ThruOsc> ThruOscPtr;

class ThruOsc : public BasicOsc {
    float _nextSample = 0.f;
    size_t _length;

public:
    ThruOsc(size_t length=1) : _length(length) {}

    void setNext(float sample) {
        _nextSample = sample;
    }

    float next() override {
        return _nextSample;
    }

    // not sure what to do with this yet...
    // actually, should pull out seperate
    // baseclass for envelopes
    size_t length() override { std::cout << "assuming this doesn't get called" << std::endl; return _length; }
    void restart() override { ; }
};

/*
 * Basic wavetable oscillator
 */

class WTFOsc;
typedef std::shared_ptr<WTFOsc> WTFOscPtr;

class WTFOsc : public BasicOsc {
    WaveTablePtr _wavetable;
    float _freq;
    size_t _sampleRate;
    float _idx;
    float _inc;
    bool _dirty = false;

public:
    WTFOsc() {};
    WTFOsc(WaveTablePtr wavetable, float freq, int sampleRate=44100, float start=0.f) :
        _wavetable(wavetable),
        _freq(freq),
        _sampleRate(sampleRate),
        _idx(start),
        _inc(wavetable->size() * _freq / _sampleRate) {}

    inline void wavetable(WaveTablePtr wavetable) {
        _wavetable = wavetable;
        if(_idx >= _wavetable->size()) {
            _idx = 0.f;
        }
        _dirty = true;
    }

    inline void freq(float f) {
        _freq = f;
        _dirty = true;
    }

    void sampleRate(size_t sampleRate) {
        _sampleRate = sampleRate;
        _dirty = true;
    }

    inline size_t length() override { return _wavetable->size(); }
    inline void restart() override { _idx = 0; }
    inline float next() override { return next(0.f); }
    inline float next(float nudgeInc) {
        if(_dirty) {
            _cookParams();
            _dirty = false;
        }
        float out = _wavetable->atF(_idx);

        _inc += nudgeInc;
        _idx += _inc;
        size_t len = _wavetable->size();
        if(_idx >= len) {
            _idx -= len;
        }
        
        return out;
    }

    inline void inc(float i) {
        _inc = i;
    }

protected:    
    inline void _cookParams() {
        size_t size = _wavetable->size();
        _inc = size * _freq / _sampleRate;
        if(_idx >= size) {
            _idx = 0;
        }
    }
};

class LUTEnvelope;
typedef std::shared_ptr<LUTEnvelope> LUTEnvelopePtr;

class LUTEnvelope : public BasicOsc {
    LUTPtr _lut;
    size_t _idx = 0;

    size_t _length;
    size_t _envRampLength;
    size_t _envRampTwo;
    float _envPhase;
    float _envPhaseInc;
    bool _dirty = false;

public:
    LUTEnvelope() {}
    LUTEnvelope(LUTPtr lut, size_t length, float envRampLengthPct=0.2f) :
        _lut(lut),
        _length(length),
        _envRampLength(length * envRampLengthPct),
        _envRampTwo(length - _envRampLength),
        _envPhase(lut->firstX()),
        _envPhaseInc((lut->lastX() - lut->firstX()) / (_envRampLength * 2 -1))
    {
        // std::cout << "building the env. x0: " << lut->firstX() << ", xN: " << lut->lastX() << std::endl;
    }

    inline void _cookParams() {
        _envRampTwo = _length - _envRampLength;
        float x0 = _lut->firstX();
        float xN = _lut->lastX();
        _envPhaseInc = (xN - x0) / (_envRampLength * 2 - 1);
        _envPhase = x0;
        if(_idx < _envRampLength) {
            _envPhase += _envPhaseInc * _idx;
        } else if(_idx < _envRampTwo) {
            _envPhase += _envPhaseInc * _envRampLength;
        } else {
            _envPhase += _envPhaseInc * (_envRampLength + _idx - _envRampTwo);
        }
    }

    inline void lut(LUTPtr lut) {
        _lut = lut;
        _dirty = true;
    }

    inline size_t length() override { return _length; }
    inline void length(size_t length) {
        _length = length;
        _dirty = true;
    }

    // pct should be <= 0.5.
    inline void envRampLength(float pct) {
        _envRampLength = _length * pct;
        _dirty = true;
    }

    inline bool atEnd() { return _idx >= _length; }

    inline float next() override {
        if(_dirty) {
            _cookParams();
            _dirty = false;
        }
        
          float out = 1.f; // 1.f is the value for the middle of the envelope
        if(_idx < _length) {
            if(_idx < _envRampLength) {
                // ramp up
                out = _lut->at(_envPhase);
                _envPhase += _envPhaseInc;

            } else if(_idx >= _envRampTwo) {
                // ramp down
                out = _lut->at(_envPhase);
                _envPhase += _envPhaseInc;
            }
            _idx++;

        } else {
            //TODO !! this shouldn't be getting hit, but does sometimes when stuff gets wonky
            std::cout << "bailing, _idx has gone too far: " << _idx << ", length: " << _length << std::endl;
            out = 0.f;
        }
        return out;
    }

    void restart() override {
        _idx = 0;
        _envPhase = _lut->firstX();
    }
};

// ---------------------- Dust Bin --------------------------------------------
// class WTIOsc {
//     WaveTablePtr _wavetable;
//     size_t _idx = 0;

// public:
//     WTIOsc(WaveTablePtr wavetable, size_t start=0) :
//         _wavetable(wavetable),
//         _idx(start) {}

//     inline float next() {
//         float out = _wavetable->at(_idx);
//         if(++_idx >= _wavetable->size()) {
//             _idx = 0;;
//         }
//         return out;
//     }
// };


#endif
