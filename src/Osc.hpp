/**
 * @file Osc.hpp
 * @author Daniel Billotte
 * @brief 
 * @version 0.1
 * @date 2021-08-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef OSC_HPP
#define OSC_HPP

#include "constants.hpp"

/**
 * @brief Base class for oscillators that operate off of a phase cycle of 2π.
 * 
 * OscBase is meant to be a starting point that has the core plumbing taken care
 * of so that different waveforms or waveform generation methods can be
 * explored.
 */
class OscBase {    
protected:
    int _sampleRate = 0;
    float _freq = 0.f;
    float _phaseInc = 0.f;
    float _phase = 0.f;
    
public:
    /**
     * @brief Set the oscillator frequency and recalculate the phase increment
     * 
     * @param freq : frequency in Hz
     */
    void setFreq(float freq) {
        if(_freq != freq) {
            _freq = freq;
            _calcPhaseInc();
        }
    }
    
    /**
     * @brief Set the sample-rate and recalculate the phase increment
     * 
     * @param sampleRate : sample-rate in samples/second
     */
    void setSampleRate(int sampleRate) {
        if(_sampleRate != sampleRate) {
            _sampleRate = sampleRate;
            _calcPhaseInc();
        }
    }

    /**
     * @brief Set the frequency and sample-rate, then recalculate the phase increment
     * 
     * @param freq : frequency in Hz
     * @param sampleRate : sample-rate in samples/second
     */
    void setPhaseParams(float freq, int sampleRate) {
        _freq = freq;
        _sampleRate = sampleRate;
        _calcPhaseInc();
    }
    
protected:
    void _calcPhaseInc() {
        _phaseInc = _twoPI * _freq / _sampleRate;
        _initCycle();
    }
    
    void _cycle() {
        _phase += _phaseInc;
        if(_phase > _twoPI)
            _phase -= _twoPI;
    }
    
    virtual void _initCycle() { ; }
};


//--------------------------------------------------------
class SingleOsc : public OscBase {
public:
    float getValue() {
        float out = _getValue();
        _cycle();
        return out;
    }
protected:
    virtual float _getValue() = 0;
};

//--------------------------------------------------------
class SineOsc : public SingleOsc {
protected:
    float _getValue() override {
        return sin(_phase);
    }
};


//--------------------------------------------------------
class TriangleOsc : public SingleOsc {
protected:
    float _getValue() override {
        float tri_out = _phase * 2.f / _PI;
        
        if(_phase < _PI/2.f) {
            //tri_out = 0.f + tri_out;
        } else if(_phase < _3halfPI) {
            tri_out = 2.f - tri_out;
        } else {
            tri_out = -4.f + tri_out;
        }
        
        return tri_out;
    }
};

//--------------------------------------------------------
class SawOsc : public SingleOsc {
protected:
    float _getValue() override {
        return _phase/_PI - 1.f;
    }
};

//--------------------------------------------------------
class SquareOsc : public SingleOsc {
protected:
    float _getValue() override {
        return (_phase < _PI) ? -1.f : 1.f;
    }
};

//--------------------------------------------------------
enum SineSawVoices {
    VOICE_SINE,
    VOICE_SAW,
    NUM_VOICES
};

//--------------------------------------------------------
template<class T>
class MultipleVoiceOsc : public OscBase {
public:
    float getValue(T type) { return _getValue(type); }
    int numVoices() { return T::NUM_VOICES; }
    void next() { _cycle(); }
    
protected:
    virtual float _getValue(T type) = 0;
};

//--------------------------------------------------------
class SineSawOsc : public MultipleVoiceOsc<SineSawVoices> {
protected:
    float _sin_out = 0.f;

    void _initCycle() override {
        _sin_out = sin(_phase);
    }

    float _getValue(SineSawVoices type) override {
        if(type == VOICE_SINE)
            return _sin_out;
           
        // type == VOICE_SAW
        return _phase/_PI - 1.f;
    }
};


#endif






