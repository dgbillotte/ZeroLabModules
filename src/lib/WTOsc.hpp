#ifndef WAVE_TABLE2_HPP
#define WAVE_TABLE2_HPP

#include "WaveTable.hpp"

// #include <mutex>
// #include <vector>



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

    inline float next() {

        // 1.f is the value for the middle of the envelope
        float out = 1.f;
        if(_idx >= _length) {
            std::cout << "bailing, _idx has gone too far: " << _idx << ", length: " << _length << std::endl;
            return 0.f;
        }
        // if(_envPhase > 2*_envRampLength) {
        //     std::cout << "bailing, _envPhase has gone too far" << std::endl;
        //     return out;
        // }
//   try {
//         std::cout << "phase: " << _envPhase << std::endl;
        if(_idx < _envRampLength) {
            // ramp up
            // std::cout << "down up: " << _envPhase << "idx: " << _envRampTwo << std::endl;
            out = _lut->at(_envPhase);
            _envPhase += _envPhaseInc;

        } else if(_idx >= _envRampTwo) {
            // ramp down
            // std::cout << "down ramp: " << _envPhase << "idx: " << _idx << std::endl;
            out = _lut->at(_envPhase);
            _envPhase += _envPhaseInc;
        }
//   }
//   catch (const std::out_of_range& oor) {
//     std::cerr << "Out of Range error: " << oor.what() << '\n';
//   }
        // std::cout << "poke 2" << std::endl;

        _idx++;
        return out;
    }
};

#endif
