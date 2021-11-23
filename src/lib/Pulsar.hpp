#ifndef PULSAR_HPP
#define PULSAR_HPP

#include "ObjectStore.hpp"
#include "WTOsc.hpp"

/*
 * Todos:
 * - tune the parameter(+CV) limits
 * 
 * Ideas:
 * - allow for overlapping trains. Each still initiated by the trigger
 *   but instead of ending on the next trigger, allowed to play out 
 *   while a *new* train is started in response to the trigger
 */
class Pulsar;
typedef std::shared_ptr<Pulsar> PulsarPtr;

class Pulsar {
    BasicOscPtr _waveOsc;
    LUTEnvelopePtr _env;
    
    size_t _idx = 0;
    size_t _length;
    float _duty;
    bool _endOfCycle = false;

    // these were just for debugging, but are computed anyways, so we get them for free
    float _lastEnv = 0;
    float _lastWav = 0;


public:

    Pulsar(BasicOscPtr osc, LUTEnvelopePtr env, size_t p, float duty) :
        _waveOsc(osc),
        _env(env),
        _length(p),
        _duty(duty)
    {
        _env->length(p * duty); 
    }


    ~Pulsar() {
        // std::cout << "Pulsar Death. Cycles left: " << _length - _idx << std::endl;
    }

    void setOsc(BasicOscPtr osc) {
        _waveOsc = osc;
    }

    void p(size_t p) {
        if(_length == p)
            return;

        _length = p;
        _env->length(_length * _duty);
    }

    // for now should be in [0..1]. 
    inline void duty(float d) {
        if(_duty == d)
            return;

        _duty = d;
        _env->length(_length * _duty);
    }

    inline size_t length() { return _length; }
    
    inline bool endOfCycle() { return _endOfCycle; }

    inline float nextSample() {
        _endOfCycle = false;
        float out = 0.f;

        if(! _env->atEnd()) { // this is emission period
            _lastWav = _waveOsc->next();
            _lastEnv = _env->next();
            out = _lastWav * _lastEnv;

        } else { // this is the sleep period
            _lastWav = 0.f;
            _lastEnv = 0.f;
            out = 0.f;
        }

        // _idx++;
        if(++_idx >= _length) { // restart the envelope
            this->_idx = 0;
            _env->restart();
            _endOfCycle = true;
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