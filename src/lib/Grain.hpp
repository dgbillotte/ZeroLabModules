#ifndef GRAIN_HPP
#define GRAIN_HPP

#include "ObjectStore.hpp"

class Grain {
    const int LUT_SIZE;
    LUTPtr _envLUT;

    const int SIN_WT_SIZE;
    WaveTablePtr _sinWT;

    int _length;
    int _idx = 0;
    float _phase = 0;
    float _phaseInc;

    int _envType;
    int _envRampLength;
    int _envRampTwo;

    float _lastEnv = 0;
    float _lastWav = 0;


public:
	enum EnvelopeTypes {
        ENV_PSDO_GAUSS,
        ENV_SINC2,
        ENV_SINC3,
        ENV_SINC4,
        ENV_SINC5,
        ENV_SINC6,
        ENV_RAMP,
		NUM_ENV_TYPES
	};

    void dump() {
        std::cout << "_length: " << _length << std::endl <<
            "_idx: " << _idx << std::endl;
    }

    float _envPhase;
    float _envPhaseInc;


    Grain(float freq, int length, int sampleRate=44100, float envRampLengthPct=0.2f, int envType=ENV_RAMP) :
        LUT_SIZE(1024),
        SIN_WT_SIZE(1024),
        _length(length),
        _phaseInc(2.f * M_PI * freq / sampleRate),
        _envType(envType),
        _envRampLength(length * envRampLengthPct),
        _envRampTwo(length - _envRampLength - 1)
    {
        auto store = ObjectStore::getStore();

        // create the envelopes
        auto sincF = [](float x) { float t=x*M_PI; return sin(t)/t; };

        if(_envType == ENV_PSDO_GAUSS) {
            _envLUT = store->loadLUT("COS_0_2PI_1024", 0.f, 2.f*M_PI, LUT_SIZE, [](float x) { return (sin(x)+1.f)/2.f; });    
        } else if(_envType == ENV_SINC2) {
            _envPhase = -2.f;
            _envPhaseInc = 4.f / _length;
            _envLUT = store->loadLUT("SINC_-2_2_1024", -2.f, 2.f, LUT_SIZE, sincF);
        } else if(_envType == ENV_SINC3) {
            _envPhase = -3.f;
            _envPhaseInc = 6.f / _length;
            _envLUT = store->loadLUT("SINC_-3_3_1024", -3.f, 3.f, LUT_SIZE, sincF);
        } else if(_envType == ENV_SINC4) {
            _envPhase = -4.f;
            _envPhaseInc = 8.f / _length;
            _envLUT = store->loadLUT("SINC_-4_4_1024", -4.f, 4.f, LUT_SIZE, sincF);
        } else if(_envType == ENV_SINC5) {
            _envPhase = -5.f;
            _envPhaseInc = 10.f / _length;
            _envLUT = store->loadLUT("SINC_-5_5_1024", -5.f, 5.f, LUT_SIZE, sincF);
        } else if(_envType == ENV_SINC6) {
            _envPhase = -6.f;
            _envPhaseInc = 12.f / _length;
            _envLUT = store->loadLUT("SINC_-6_6_1024", -6.f, 6.f, LUT_SIZE, sincF);

        }



        // create the wavetables
        _sinWT = store->loadWavetable("SIN_0_2PI_1024", 0.f, 2.f*M_PI, SIN_WT_SIZE, sin);
        // auto sqrF = 
        // _sinWT = store->loadWavetable("SIN_0_2PI_1024", 0.f, 2.f*M_PI, SIN_WT_SIZE, []);

    }

    float envOut() { return _lastEnv; }
    float wavOut() { return _lastWav; }

    ~Grain() {
        // std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    float nextSample() {
        if(_idx < _length) {
            _lastWav = _nextWaveSample();
            _lastEnv = _nextEnvelopeValue();
            return _lastWav * _lastEnv;
        }
        return 0.f;
    }

    bool running() { return _idx < _length; }
    void restart() { _idx = 0; }

protected:

    float _nextWaveSample() {

        float out = _sinWT->at(_phase);
        // std::cout << "got values: " << out << ", " << out2 << std::endl;

        _phase += _phaseInc;
        if(_phase >= 2.f * M_PI) {
            _phase -= 2.f * M_PI;
        }

        return out;
    }

    float _nextEnvelopeValue() {
        float out = 1.f;
        
        if(_envType == ENV_PSDO_GAUSS) {
            float rampInc = M_PI / _envRampLength;
            if(_idx < _envRampLength) {
                float theta = M_PI + (_idx * rampInc);
                out = _envLUT->at(theta);
            } else if(_idx >= _envRampTwo) {
                float theta = (_idx - _envRampTwo) * rampInc;
                out = _envLUT->at(theta);
            }
            // else out = 1.0f from init

        } else if(_envType < ENV_RAMP) {
            out = _envLUT->at(_envPhase);
            _envPhase += _envPhaseInc;

        } else { // ENV_RAMP
            float mid = _length / 2;
            out = (_idx <= mid) ? _idx / mid : 1 - (_idx-mid) / mid;
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