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
    LUTPtr _envLUT;
    WaveTablePtr _wavetable;
    WTFOsc& _waveOsc;
    LUTEnvelope& _env;

    int _length;
    int _idx = 0;

 
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


    Grain(WTFOsc& osc, LUTEnvelope& env) :
        _waveOsc(osc),
        _env(env),
        _length(env.length())
    {}

    // Grain() : _waveOsc(), _env() {};


    float envOut() { return _lastEnv; }
    float wavOut() { return _lastWav; }

    ~Grain() {
        // std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    inline float nextSample() {
        if(_idx++ < _length) {
            _lastWav = _waveOsc.next();
            _lastEnv = _env.next();
            return _lastWav * _lastEnv;
        }
        return 0.f;
    }

    bool running() { return _idx < _length; }
    void restart() { _idx = 0; }

};


#endif