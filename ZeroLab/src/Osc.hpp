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

#include "AudioLib.hpp"


typedef std::function<float(float)> PhaseToSample;


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

class SingleOsc2 : public SingleOsc {
public:
    SingleOsc2() : _generator { _IDENTITY_F } {}
    SingleOsc2(PhaseToSample &generator) : _generator { generator } {}

    void setGenerator(PhaseToSample &generator) {
        _generator = generator;
    }

    float _getValue() {
        return _generator(_phase);
    }

protected:
    PhaseToSample& _generator;
    PhaseToSample _IDENTITY_F = [](float phase){ return phase; };
};


//--------------------------------------------------------
// With SingleOsc2 subclasses can be very simple
PhaseToSample sinGenerator = [](float _phase) { return sin(_phase); };
class SineOsc2 : public SingleOsc2 {
    SineOsc2() : SingleOsc2{sinGenerator} {}
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

PhaseToSample triangleGenerator = [](float phase) {
    float tri_out = phase * 2.f / _PI;

    if(phase < _PI/2.f) {
        return tri_out;
    } else if(phase < _3halfPI) {
        return 2.f - tri_out;
    } else {
        return -4.f + tri_out;
    }
};

class TriangleOsc2 : public SingleOsc2 {
    TriangleOsc2() : SingleOsc2{ triangleGenerator } {};
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



class DIYQuadrant : public SingleOsc {
public:
    void setQ1(PhaseToSample f) { _q1 = f; }
    void setQ2(PhaseToSample f) { _q2 = f; }
    void setQ3(PhaseToSample f) { _q3 = f; }
    void setQ4(PhaseToSample f) { _q4 = f; }
    
    void setAllQ(PhaseToSample f1,
                 PhaseToSample f2,
                 PhaseToSample f3,
                 PhaseToSample f4) {
        _q1 = f1;
        _q2 = f2;
        _q3 = f3;
        _q4 = f4;
    }


protected:
    float _getValue() override {
        float out = 0.f;
        if(_phase < _PI/2.f)
            out = _q1(_phase);
        else if(_phase < _PI)
            out = _q2(_phase);
        else if(_phase < _3halfPI)
            out = _q3(_phase);
        else
            out = _q4(_phase);
        return clamp(out, -1.f, 1.f);
    }
    
    PhaseToSample _q1;
    PhaseToSample _q2;
    PhaseToSample _q3;
    PhaseToSample _q4;
};
//--------------------------------------------------------
class KindaEvenOsc : public DIYQuadrant {
public:
    KindaEvenOsc() {
        setAllQ([](float _phase){ return sin(_phase); },
                [](float _phase){ return (-sin(_phase)*0.5f) + 0.2f; },
                [](float _phase){ return (-sin(_phase)*0.5f) - 0.2f; },
                [](float _phase){ return sin(_phase); }
                );
    }
};

//--------------------------------------------------------
// diff implementation of above. instead of using the
// diyquadrant,
PhaseToSample quadGen(PhaseToSample& q1,
                    PhaseToSample& q2,
                    PhaseToSample& q3,
                    PhaseToSample& q4) {
    return [q1, q2, q3, q4](float phase) {
        float out = 0.f;
        if(phase < _PI/2.f)
            out = q1(phase);
        else if(phase < _PI)
            out = q2(phase);
        else if(phase < _3halfPI)
            out = q3(phase);
        else
            out = q4(phase);
        return out;
    };
};

// PhaseToSample keGen = quadGen([](float _phase){ return sin(_phase); },
//                 [](float _phase){ return (-sin(_phase)*0.5f) + 0.2f; },
//                 [](float _phase){ return (-sin(_phase)*0.5f) - 0.2f; },
//                 [](float _phase){ return sin(_phase); }
//                 );
// class KindaEven2 : public SingleOsc2 {
//     KindaEven2() : SingleOsc2{ keGen } {}
// };

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
enum SineSawVoices {
    VOICE_SINE,
    VOICE_SAW,
    NUM_VOICES
};

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

//--------------------------------------------------------
enum PhattyVoices {
    VOICE_SINE1,
    VOICE_SAW1,
    VOICE_SQR1,
    VOICE_SINE_SUB1,
    VOICE_SQR_SUB1,
    VOICE_WHA_SUB1,
    VOICE_SINE_SUB2,
    VOICE_SQR_SUB2,
    VOICE_WHA_SUB2,
    NUM_VOICE
};


class Phatty : public MultipleVoiceOsc<PhattyVoices> {
    int _sub_cycle_count = 0;
    float _last_phase = 0.f;
    float _phase_sub1 = 0.f;
    float _phase_sub1x = 0.f;
    float _sine_sub1 = 0.f;
    float _phase_sub2 = 0.f;
    float _phase_sub2x = 0.f;
    float _sine_sub2 = 0.f;

    void _initCycle() override {
        if(_last_phase > _phase)
            if(_sub_cycle_count++ >= 3)
                _sub_cycle_count = 0;
        _last_phase = _phase;

        // this was a bug that produced a cool sound
        _phase_sub1x = (_phase + _sub_cycle_count) * _PI;
        _phase_sub2x= (_phase + _sub_cycle_count) * _halfPI;
        
        // this one is cool too
        // _phase_sub1 = _phase + (_sub_cycle_count * _PI);
        // _phase_sub2 = _phase + (_sub_cycle_count * _halfPI);
    
        // this one is correct
        _phase_sub1 = (_phase + ((_sub_cycle_count % 2) * _twoPI))/2.f;
        _phase_sub2 = (_phase + (_sub_cycle_count * _twoPI))/4.f;

    }

    float _getValue(PhattyVoices type) override {
        if(type == VOICE_SINE1) {
            return sin(_phase);
        } else if(type == VOICE_SAW1) {
            return _phase/_PI - 1.f;
        } else if(type == VOICE_SQR1) {
            return (_phase < _PI) ? -1.f : 1.f;
        } else if(type == VOICE_SINE_SUB1) {
            return  sin(_phase_sub1);
        } else if(type == VOICE_SINE_SUB2) {
            return sin(_phase_sub2);
        } else if(type == VOICE_SQR_SUB1) {
            return (_phase_sub1 < _PI) ? -1.f : 1.f;
        } else if(type == VOICE_SQR_SUB2) {
            return (_phase_sub2 < _PI) ? -1.f : 1.f;
        } else if(type == VOICE_WHA_SUB1) {
            return sin(_phase_sub1x);
        } // else VOICE_WHA_SUB2
        return sin(_phase_sub2x);
    }
};

// this was awesome as is, just in case I screw it up while working on it
// class Phatty : public MultipleVo          siceOsc<PhattyVoices> {
//     int _sub_cycle_count = -1;
//     float _last_phase = 0.f;

//     void _initCycle() {
//         if(_last_phase > _phase)
//             if(_sub_cycle_count++ > 1)
//                 _sub_cycle_count = 0;
//         _last_phase = _phase;
//     }

//     float _getValue(PhattyVoices type) override {
//         if(type == VOICE_SINE1) {
//             return sin(_phase);
//         } else if(type == VOICE_SAW1) {
//             return _phase/_PI - 1.f;
//         } else if(type == VOICE_SQR1) {
//             return (_phase < _PI) ? -1.f : 1.f;
//         } //else if(type == VOICE_SINE_SUB1) {
        
//             if(_sub_cycle_count == 0) {
//                 return sin(_phase/2.f);
//             } else {
//                 return -sin(_phase/2.f);
//             }
            

//         //}
//     }
// };







