#ifndef SMITH_ANGELL_RESONATOR_HPP
#define SMITH_ANGELL_RESONATOR_HPP

/*
 * This is an implementation of the Smith-Angell Resonator as described by
 * Pirkle in "Designing Audio Effect Plugins in C++" 1st Ed.
 * 
 * I should make this inherit from BiQuad and make it part of the Filter family.
 */

class SmithAngellResonator {
    // delay elements
    float x1=0.f, x2=0.f, y1=0.f, y2=0.f;

    // coefficients
    float a0, a2, b1, b2;

    int _sampleRate;
    float _freq;
    float _q;
    int _dirty;
    float TWO_PI = 2*M_PI;

public:
    SmithAngellResonator(int sampleRate=44100, float frequency=440.f, float q=0.f) :
        _sampleRate(sampleRate),
        _freq(frequency),
        _q(q),
        _dirty(true)
    {}

    void sampleRate(int sampleRate) {
        if(_sampleRate == sampleRate)
            return;
        _sampleRate = sampleRate;
        _dirty = true;
    }

    void freq(float freq) {
        if(_freq == freq)
            return;
        _freq = freq;
        _dirty = true;
    }

    void q(float q) {
        if(_q == q)
            return;
        _q = q;
        _dirty = true;
    }

    float process(float x0) {
        if(_dirty) {
            _calcCoefficients();
        }
        
        float y0 = (a0 * x0) + (a2 * x2) - (b1 * y1) - (b2 * y2);
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        
        return y0;
    }

protected:
    void _calcCoefficients() {
        float theta = TWO_PI * _freq / _sampleRate;
        float bw = _freq / _q;

        b2 = exp(-TWO_PI * bw / _sampleRate);
        b1 = -4 * b2 * cos(theta) / (1 + b2);
        a0 = 1 - sqrt(b2);
        a2 = -a0;
        _dirty = false;
    }
    
};


#endif