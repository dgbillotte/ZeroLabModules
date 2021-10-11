#ifndef DIRECT_OSC_HPP
#define DIRECT_OSC_HPP  
/*
 * These are both nearly direct implementations of the Direct-Form oscillator and
 * the Gordon-Smith oscillator as described by Pirkle. These are both interesting
 * because they are feedback oscillators, essentially zero-input filters that are
 * configured to self-oscillate. Additionally, while trigonometric functions are
 * required to cook the parameters when the frequency changes, none are needed
 * for each process call, instead only a couple multiplications, so they should
 * be very light-weight.
 */
#include "Filter.hpp"

//--------------------------------------------------------
// Feedback based sine oscillator
// Based on the DirectOscillator from Pirkle
class FBSineOsc : public BiQuad {
public: 
    using BiQuad::BiQuad;
    FBSineOsc(int sample_rate=44100) :
        BiQuad(440, sample_rate) {
            a0 = a1 = a2 = 0.f;
        }

    float nextSample() {
        return process(0.f);
    }
protected:
    void computeCoefficients() override {
        float wT = 2.f * M_PI *_freq / (float)_sampleRate;
        
        // calculate coefficients
        b1 = -2.f * cos(wT);
        b2 = 1.f;

        // calculate initial conditions, simple version
        y1 = sin(-1.f * wT);
        y2 = sin(-2.f * wT);

        // calculate initial conditions, better
        double t1 = asin(y1);
        float n = t1/wT;
        n += (y1 > y2) ? -1 : 1;
        y2 = sin(n * wT);

    }
};

/*
 * This seems to work well with one curiosity. The phase seems to slowly
 * oscillate back and forth. I noticed it when I was subtracting this wave
 * from one created by the oscillator above, to see how closely they matched.
 * The result is a sine wave that starts with zero amplitude, slowly
 * rises up to a peak of +/- 10V (20V pp), and then slowly goes back down 
 * to zero. The two waves are 180 deg out of phase at the peak. This repeats forever.... 
 * 
 * Guesses:
 * - X rounding error? try doubles? Tried this, it made no difference
 */
class GordonSmithOsc {
    int _sampleRate;
    float _freq;
    float _epsilon;
    float _yq;
    float _yn;
    int _dirty;

public:
    GordonSmithOsc(int sampleRate=44100, float freq=440.f) :
        _sampleRate(sampleRate),
        _freq(freq)
    {
        float wT = 2.f * M_PI * _freq / (float)_sampleRate;
        _epsilon = 2.f * sin(wT / 2.f);
        _yq = cos(-wT);
        _yn = sin(-wT);
        _dirty = true;
    };

    void freq(float freq) {
        if(_freq == freq)
            return;
        _freq = freq;
        _dirty = true;
    }

    void sampleRate(int sampleRate) {
        if(_sampleRate == sampleRate)
            return;
        _sampleRate = sampleRate;
        _dirty = true;
    }

    int _first = true;
    float nextSample() {
        if(_dirty) {
            // float wT = 2.f * M_PI * _freq / (float)_sampleRate;
            // _epsilon = 2.f * sin(wT / 2.f);
            // factored 2.f wT and its use in _epsilon
            float wT = M_PI * _freq / (float)_sampleRate;
            _epsilon = 2.f * sin(wT);
            _dirty = false;
        }

        // calculate output & set delays
        _yq -= _epsilon * _yn;
        _yn += _epsilon * _yq;

        return _yn;
    }
    // the code below more closely follows the algorithm as given
    // by Pirkle. The above is just after I messed with it a while. They
    // both function the same way as described at top.
    // float nextSample() {
    //     if(_dirty) {
    //         float wT = 2.f * M_PI * _freq / (float)_sampleRate;
    //         _epsilon = 2.f * sin(wT / 2.f);
    //         _dirty = false;
    //     }

    //     // calculate output
    //     float yqn = _yq - (_epsilon * _yn);
    //     float output = _yn + (_epsilon * yqn);

    //     // set delays
    //     _yq = yqn;
    //     _yn = output;

    //     return output;
    // }
};


#endif