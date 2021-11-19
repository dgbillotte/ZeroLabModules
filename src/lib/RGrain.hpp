#ifndef RGRAIN_HPP
#define RGRAIN_HPP

#include "ObjectStore.hpp"
#include "WTOsc.hpp"

/*
 * Todos:
 * - add repeat capability
 * - add optional end frequency for glisandos 
 */
class RGrain {
    WTFOsc& _waveOsc;
    LUTEnvelope& _env;
    
    size_t _idx = 0;
    // bool _repeat;
    size_t _repeatDelay;
    // size_t _repeatIdx = 0;
 
    // these are just for debugging, but are computed anyways, so we get them for free
    float _lastEnv = 0;
    float _lastWav = 0;


public:


    RGrain(WTFOsc& osc, LUTEnvelope& env, size_t repeatDelay=0) :
        _waveOsc(osc),
        _env(env),
        // _repeat(repeatDelay >= 0),
        _repeatDelay(repeatDelay)
    {}

    ~RGrain() {
        // std::cout << "RGrain Death. Cycles left: " << _length - _idx << std::endl;
    }


    void repeatDelay(size_t delaySamples) {
        _repeatDelay = delaySamples;
    }

    inline bool running() { return true; /*_idx < _env.length();*/ }
    
    inline float nextSample() {

        // produce out
        float out = 0.f;
        if(_idx < _env.length()) {
            _lastWav = _waveOsc.next();
            _lastEnv = _env.next();
            out = _lastWav * _lastEnv;
        } // else we are resting, so return 0.f

        _idx++;
        if(_idx >= _env.length() + _repeatDelay) {
            _idx = 0;
            _env.restart();
        }

        return out;
    }

    inline float envOut() { return _lastEnv; }
    inline float wavOut() { return _lastWav; }

    void dump() {
        std::cout << "_length: " << _env.length() << std::endl <<
            "_idx: " << _idx << std::endl;
    }
};


#endif