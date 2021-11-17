#ifndef GRAIN_HPP
#define GRAIN_HPP

#include "ObjectStore.hpp"
#include "WTOsc.hpp"

/*
 * Todos:
 * - add repeat capability
 * - add optional end frequency for glisandos 
 */
class Grain {
    const int LUT_SIZE = 1024;
    LUTPtr _envLUT;

    // const int WT_SIZE = 1024;
    WaveTablePtr _wavetable;
    WTFOscPtr _waveOsc;

    int _length;
    int _idx = 0;

    int _envType;
    int _envRampLength;
    int _envRampTwo;

    float _lastEnv = 0;
    float _lastWav = 0;


public:
    enum WaveTypes {
        WAV_SIN,
        WAV_SIN1_3_5,
        WAV_SQR,
        WAV_SIN1_2_4,
        WAV_SAW,
        NUM_WAV_TYPES
    };

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

    size_t _playIdx = 0;


    Grain(WTFOscPtr osc, int length, int sampleRate=44100, float envRampLengthPct=0.2f, int envType=ENV_RAMP) :
        _waveOsc(osc),
        _length(length),
        _envType(envType),
        _envRampLength(length * envRampLengthPct),
        _envRampTwo(length - _envRampLength - 1)    
    {
        _loadEnvelope(envType);
    }

    Grain(float freq, int length, int sampleRate=44100, float envRampLengthPct=0.2f, int envType=ENV_RAMP, int waveType=WAV_SIN, float finalFreq=-1) :
        _length(length),
        _envType(envType),
        _envRampLength(length * envRampLengthPct),
        _envRampTwo(length - _envRampLength - 1)
    {
        // _loadOsc(freq, sampleRate, waveType);
        // create the envelopes
        _loadEnvelope(envType);
    }


    void _loadEnvelope(int envType) {
        auto waveBank = ObjectStore::getStore();
        auto sincF = [](float x) { float t=x*M_PI; return sin(t)/t; };

        if(_envType == ENV_PSDO_GAUSS) {
            _envPhase = -M_PI;
            _envPhaseInc = M_PI / _envRampLength;
            _envLUT = waveBank->loadLUT("COS_0_2PI_1024", -M_PI, M_PI, LUT_SIZE, [](float x) { return (cos(x)+1.f)/2.f; }, false);    
        } else if(_envType == ENV_SINC2) {
            _envPhase = -2.f;
            _envPhaseInc = 2.f / _envRampLength;
            _envLUT = waveBank->loadLUT("SINC_-2_2_1024", -2.f, 2.f, LUT_SIZE, sincF, false);
        } else if(_envType == ENV_SINC3) {
            _envPhase = -3.f;
            _envPhaseInc = 3.f / _envRampLength;
            _envLUT = waveBank->loadLUT("SINC_-3_3_1024", -3.f, 3.f, LUT_SIZE, sincF, false);
        } else if(_envType == ENV_SINC4) {
            _envPhase = -4.f;
            _envPhaseInc = 4.f / _envRampLength;
            _envLUT = waveBank->loadLUT("SINC_-4_4_1024", -4.f, 4.f, LUT_SIZE, sincF, false);
        } else if(_envType == ENV_SINC5) {
            _envPhase = -5.f;
            _envPhaseInc = 5.f / _envRampLength;
            _envLUT = waveBank->loadLUT("SINC_-5_5_1024", -5.f, 5.f, LUT_SIZE, sincF, false);
        } else if(_envType == ENV_SINC6) {
            _envPhase = -6.f;
            _envPhaseInc = 6.f / _envRampLength;
            _envLUT = waveBank->loadLUT("SINC_-6_6_1024", -6.f, 6.f, LUT_SIZE, sincF, false);
        }
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
        return _waveOsc->next();
    }

    float _nextEnvelopeValue() {
        float out = 1.f;
        
        if(_envType < ENV_RAMP) {
            if(_idx < _envRampLength) {
                out = _envLUT->at(_envPhase);
                _envPhase += _envPhaseInc;

            } else if(_idx >= _envRampTwo) {
                out = _envLUT->at(_envPhase);
                _envPhase += _envPhaseInc;
            }

        } else { // ENV_RAMP
            float mid = _length / 2;
            out = (_idx <= mid) ? _idx / mid : 1 - (_idx-mid) / mid;
        }

        _idx++;
        return out;
    }

};


#endif