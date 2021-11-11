#ifndef GRAIN_HPP
#define GRAIN_HPP

#include "Util.hpp"


const int COS_LUT_SIZE = 1000;
const int COS_RAMP_SIZE = COS_LUT_SIZE / 2;
CosLUT cosLUT(COS_LUT_SIZE);


class Grain {
    int _length = 0;
    int _idx = 0;
    float _phase = 0.f;
    float _phaseInc = 0.f;

    int _envRampLength;
    float _envInc;
    int _envRampTwo;

    // float lastEnv = 0;
    // float lastWav = 0;

    // static CosLUT cosLUT;

public:
    void dump() {
        std::cout << "_length: " << _length << std::endl <<
            "_idx: " << _idx << std::endl;
    }

    Grain(float freq, int length, int sampleRate=44100) :
        _length(length),
        _phaseInc(2.f * M_PI * freq / sampleRate),
        _envRampLength(length * 0.5f),
        _envInc(COS_RAMP_SIZE / _envRampLength),
        _envRampTwo(length - _envRampLength - 1)
    {}

    Grain(const Grain& grain) :
        _length(grain._length),
        _idx(grain._idx),
        _phase(grain._phase),
        _phaseInc(grain._phaseInc),
        _envRampLength(grain._envRampLength),
        _envInc(grain._envInc),
        _envRampTwo(grain._envRampTwo)
    {}

    // float envOut() { return lastEnv; }
    // float wavOut() { return lastWav; }

    ~Grain() {
        // std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    float nextSample() {
        if(_idx < _length) {
            // grain is running
            return _nextWaveSample() * _nextEnvelopeValue();
        } else {
            std::cout << "do we get here?" << std::endl;
        }
        return 0.f;
    }

    bool running() { return _idx < _length; }
    void restart() { _idx = 0; }

protected:
    float _nextWaveSample() {
        float out = sin(_phase);

        _phase += _phaseInc;
        if(_phase >= 2.f * M_PI) {
            _phase -= 2.f * M_PI;
        }

        return out;
    }


    float _nextEnvelopeValue() {
        float out = 1.f;
        if(_idx < _envRampLength) {
            float theta = (COS_RAMP_SIZE + _idx * _envInc);
            out = (cosLUT.at(theta) + 1.f) / 2.f;
        } else if(_idx >= _envRampTwo) {
            float theta = (_idx - _envRampTwo) * _envInc;
            out = (cosLUT.at(theta) + 1.f) / 2.f;
        }
        _idx++;
        return out;
    }

    // the OG triangle. It worked great, until it didn't
    // float _nextEnvelopeValue() {
    //     float mid = _length / 2;
    //     float out = (_idx <= mid) ? _idx / mid : 1 - (_idx-mid) / mid;
    //     _idx++;
    //     return out;
    // }



};


#endif