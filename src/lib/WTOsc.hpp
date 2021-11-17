#ifndef WAVE_TABLE2_HPP
#define WAVE_TABLE2_HPP

#include "WaveTable.hpp"



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

class WTFOsc;
typedef std::shared_ptr<WTFOsc> WTFOscPtr;

class WTFOsc {
    WaveTablePtr _wavetable;
    float _freq;
    float _sampleRate;
    float _idx;
    float _inc;

public:
    WTFOsc() {};
    WTFOsc(WaveTablePtr wavetable, float freq, int sampleRate=44100, float start=0.f) :
        _wavetable(wavetable),
        _freq(freq),
        _sampleRate(sampleRate),
        _idx(start),
        _inc(wavetable->size() * _freq / _sampleRate) {}

    void freq(float f) {
        _freq = f;
        _inc = _wavetable->size() * _freq / _sampleRate;
    }

    inline float next(float nudgeInc=0.f) {
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
};

class LUTEnvelope;
typedef std::shared_ptr<LUTEnvelope> LUTEnvelopePtr;

class LUTEnvelope {
    LUTPtr _lut;
    size_t _idx = 0;

    size_t _length;
    size_t _envRampLength;
    size_t _envRampTwo;
    float _envPhase;
    float _envPhaseInc;


public:
    LUTEnvelope(LUTPtr lut, size_t length, size_t sampleRate, float envRampLengthPct=0.2f) :
        _lut(lut),
        _length(length),
        _envRampLength(length * envRampLengthPct),
        _envRampTwo(length - _envRampLength),
        _envPhase(lut->firstX()),
        _envPhaseInc((lut->lastX() - lut->firstX()) / (_envRampLength * 2 -1))
    {
        // std::cout << "building the env. x0: " << lut->firstX() << ", xN: " << lut->lastX() << std::endl;
    }

    inline size_t length() { return _length; }

    inline float next() {
        if(_idx >= _length) {
            //TODO !! this shouldn't be getting hit, but is
            // std::cout << "bailing, _idx has gone too far: " << _idx << ", length: " << _length << std::endl;
            return 0.f;
        }

        // 1.f is the value for the middle of the envelope
        float out = 1.f;
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
        return out;
    }
};

#endif
