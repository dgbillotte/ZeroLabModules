#ifndef GRAIN_HPP
#define GRAIN_HPP

#include "Util.hpp"
#include "ObjectStore.hpp"

class Grain {
    const int COS_LUT_SIZE;
    // const int COS_RAMP_SIZE;
    LUTPtr _cosLUT;

    int _length;
    int _idx = 0;
    float _phase;
    float _phaseInc;

    int _envRampLength;
    float _envInc;
    int _envRampTwo;

    float _lastEnv = 0;
    float _lastWav = 0;

    // LUT cosLUT = LUT(0.f, 2.f*M_PI, 1000, cos);

    // static CosLUT cosLUT;

public:
    void dump() {
        std::cout << "_length: " << _length << std::endl <<
            "_idx: " << _idx << std::endl;
    }

    Grain(float freq, int length, int sampleRate=44100, float envRampLengthPct=0.2f) :
        COS_LUT_SIZE(1024),
        // COS_RAMP_SIZE(COS_LUT_SIZE/2),
        _cosLUT(ObjectStore::getStore()->loadLUT("COS_0_2PI_1024", 0.f, 2.f*M_PI, COS_LUT_SIZE, [](float x) { return (cos(x)+1.f)/2.f; })),
        _length(length),
        _phaseInc(2.f * M_PI * freq / sampleRate),
        _envRampLength(length * envRampLengthPct),
        // _envInc(COS_RAMP_SIZE / _envRampLength),
        _envInc(M_PI / _envRampLength),
        _envRampTwo(length - _envRampLength - 1)
    {}

    float envOut() { return _lastEnv; }
    float wavOut() { return _lastWav; }

    ~Grain() {
        // std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    float nextSample() {
        if(_idx < _length) {
            // grain is running
            _lastWav = _nextWaveSample();
            _lastEnv = _nextEnvelopeValue();
            return _lastWav * _lastEnv;
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
            float theta = M_PI + (_idx * _envInc);
            out = _cosLUT->at(theta);
        } else if(_idx >= _envRampTwo) {
            float theta = (_idx - _envRampTwo) * _envInc;
            out = _cosLUT->at(theta);
        }
        // std::cout << "env: " << out << std::endl;
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