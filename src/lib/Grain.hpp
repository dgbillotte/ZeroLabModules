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
    WTFOsc& _waveOsc;
    LUTEnvelope& _env;
    
    size_t _idx = 0;
    bool _repeat = false;
    size_t _repeatDelay;

    // these are just for debugging, but are computed anyways, so we get them for free
    float _lastEnv = 0;
    float _lastWav = 0;


public:


    Grain(WTFOsc& osc, LUTEnvelope& env, int repeatDelay=-1) :
        _waveOsc(osc),
        _env(env),
        _repeat(repeatDelay >= 0),
        _repeatDelay(_repeat ? repeatDelay : 0)
    {}

    ~Grain() {
        // std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    void repeatDelay(size_t delaySamples) {
        _repeatDelay = delaySamples;
    }

    bool running() { return _repeat || _idx < _env.length(); }
    
    // void restart() { _idx = 0; }

    inline float nextSample() {
        float out = 0.f;

        if(_repeat) {
            if(_idx < _env.length()) {
                _lastWav = _waveOsc.next();
                _lastEnv = _env.next();
                out = _lastWav * _lastEnv;
            } 

            _idx++;
            if(_idx >= _env.length() + _repeatDelay) {
                _idx = 0;
                _env.restart();
            }

        } else {
            if(_idx < _env.length()) {
                _lastWav = _waveOsc.next();
                _lastEnv = _env.next();
                _idx++;
                out = _lastWav * _lastEnv;
            } 
        }

        return out;
    }

    float envOut() { return _lastEnv; }
    float wavOut() { return _lastWav; }

    void dump() {
        std::cout << "_length: " << _env.length() << std::endl <<
            "_idx: " << _idx << std::endl;
    }
};


#endif