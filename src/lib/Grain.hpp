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
 
    // these are just for debugging, but are computed anyways, so we get them for free
    float _lastEnv = 0;
    float _lastWav = 0;


public:


    Grain(WTFOsc& osc, LUTEnvelope& env) :
        _waveOsc(osc),
        _env(env)
    {}

    ~Grain() {
        // std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    bool running() { return _idx < _env.length(); }
    
    void restart() { _idx = 0; }

    inline float nextSample() {
        float out = 0.f;
        if(_idx < _env.length()) {
            _lastWav = _waveOsc.next();
            _lastEnv = _env.next();
            // std::cout << "grain-idx: " << _idx << ", wave: " << _lastWav << ", env: " << _lastEnv << std::endl << std::endl;
            _idx++;
            out = _lastWav * _lastEnv;
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