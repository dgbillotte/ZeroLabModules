#ifndef PULSAR_HPP
#define PULSAR_HPP

#include "ObjectStore.hpp"
#include "WTOsc.hpp"

/*
 * Todos:
 * - the internals of this still reflect that it was converted from Grain. Refactor
 *   it to remove _repeatDelay and instead work in terms of p and duty.
 */
class Pulsar {
    BasicOscPtr _waveOsc;
    LUTEnvelopePtr _env;
    
    size_t _idx = 0;
    float _duty;
    size_t _repeatDelay;

    // these are just for debugging, but are computed anyways, so we get them for free
    float _lastEnv = 0;
    float _lastWav = 0;


public:

    /*
     * Notes:
     * - total length is defined by the LUTEnvelope, may want to internalize it...
     */ 

    Pulsar(BasicOscPtr osc, LUTEnvelopePtr env, size_t p, float duty) :
        _waveOsc(osc),
        _env(env),
        _duty(duty),
        _repeatDelay(p * (1.f - duty))
    {
        _env->length(p - _repeatDelay);
    }



    ~Pulsar() {
        // std::cout << "Pulsar Death. Cycles left: " << _length - _idx << std::endl;
    }

    void setOsc(BasicOscPtr osc) {
        _waveOsc = osc;
    }

    void p(size_t p) {
        _repeatDelay = p * (1.f - _duty);
        _env->length(p * _duty);
        // std::cout << "p: " << p << ", duty: " << _duty << ", delay: " << _repeatDelay << ", envLen: " << p - _repeatDelay << std::endl;
    }

    // for now should be in [0..1]. 
    void duty(float d) {
        if(_duty == d)
            return;
        _duty = d;
        // casting to int in the math below always loses 1, so add it to the 
        size_t len = length();
        _repeatDelay = len * (1.f - _duty);
        _env->length(len - _repeatDelay);
        // _env.length(len * _duty);
        // std::cout << "duty change: _duty: " << _duty << ", len0: " << len << ", len1: " << length() << std::endl;
    }

    inline size_t length() { return _env->length() + _repeatDelay; }
    
    inline bool endOfCycle() { return _firstSample; }

    bool _firstSample = false;
    inline float nextSample() {
        _firstSample = false;
        float out = 0.f;

        if(! _env->atEnd()) { // this is actual "grain" 
            _lastWav = _waveOsc->next();
            _lastEnv = _env->next();
            out = _lastWav * _lastEnv;

        } else { // this is the sleep or repeat-delay period
            _lastWav = 0.f;
            _lastEnv = 0.f;
            out = 0.f;
        }

        _idx++;
        if(_idx >= (_env->length() + _repeatDelay)) { // restart the envelope
            this->_idx = 0;
            _env->restart();
            _firstSample = true;
        }

        return out;
    }

    float envOut() { return _lastEnv; }
    float wavOut() { return _lastWav; }

    void dump() {
        std::cout << "_length: " << _env->length() << std::endl <<
            "_idx: " << _idx << std::endl;
    }
};


#endif