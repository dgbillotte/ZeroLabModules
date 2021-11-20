#ifndef GRAIN_HPP
#define GRAIN_HPP

#include "ObjectStore.hpp"
#include "WTOsc.hpp"

/*
 * Todos:
 * - add repeat capability
 * - add optional end frequency for glisandos 
 * 
 * Current Bug in grain length:
 * - some changes to grain length cause the envelope to get stuck at 1.0
 * - does not happen as grain length increases
 * - happens sometimes as grain length decreases
 *   - small decreases don't cause it
 *   - longer continuous (mouse down) decreases trigger it
 *   - the above "length" feels predictable
 * - any change to the ramp length will properly reset it
 * - update: adjusting how and in what order length, ramp-length, and
 *   some other parameters we set fixed the bug. The ramps are still
 *   messed up during continuous changes, but stablize when the change stops
 * - update part two: moving the updating of the length and ramp-length
 *   parameters into the audio rate helped quite a bit
 */
class Grain {
    BasicOsc& _waveOsc;
    LUTEnvelope& _env;
    
    size_t _idx = 0;
    bool _repeat = false;
    size_t _repeatDelay;

    // these are just for debugging, but are computed anyways, so we get them for free
    float _lastEnv = 0;
    float _lastWav = 0;


public:


    Grain(BasicOsc& osc, LUTEnvelope& env, int repeatDelay=-1) :
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

        if(_repeat) { // repeating grain
            if(! _env.atEnd()) { // this is actual "grain" 
                _lastWav = _waveOsc.next();
                _lastEnv = _env.next();
                out = _lastWav * _lastEnv;

            } else { // this is the sleep or repeat-delay period
                _lastWav = 0.f;
                _lastEnv = 0.f;
                out = 0.f;
            }

            _idx++;
            if(_idx >= (_env.length() + _repeatDelay)) { // restart the envelope
                this->_idx = 0;
                _env.restart();
            }

        } else { // one-shot grain
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