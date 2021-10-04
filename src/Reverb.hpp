

/*
 * Schroeder's Reverb as described by Will Pirkle.
 *
 */
#ifndef REVERB
#define REVERB

#include "AllPassFilter.hpp"
#include "CombFilter.hpp"

/*
Schroeder Reverb:
This is an implementation of the Schroeder Reverb as described by Will Pirkle.

It is composed of M parallel comb filters that are summed together and then
passed through N all-pass filters serially. In the book, M=4 and N=2 but this
implementation allows the user to determine M & N. Below is a 4x2 instance.

int combf_freqs {1321, 1447, 1657, 1993};
int apf_freqs = {33, 89};
ReverbS reverb = ReverbS(combf_freqs, apf_freqs);

*/

class ReverbS {
    CombFilter** _combs;
    AllPassFilter** _apfs;

    int _nCombs;
    int _nApfs;
    float _rt60;
    float _apfG;
    int _sampleRate;
    float _dryWetMix = 0.5f;
    int _dirty = false;
    const float _wetGain = 0.75f;

public:
    ReverbS(int* comb_freqs, int* apf_freqs, float rt60=0.5f, float apf_g=0.5f, int sample_rate=44100) :
        _nCombs(0),
        _nApfs(0),
        _rt60(rt60),
        _apfG(apf_g),
        _sampleRate(sample_rate) {

        for(int i=0; comb_freqs[i] != 0; i++) {
            int freq = comb_freqs[i];
            float comb_g = combG(freq);
            _combs[i] = new CombFilter(freq, comb_g);
            _nCombs++;
        }
        

        for(int i=0; apf_freqs[i] != 0; i++) {
            int freq = apf_freqs[i];
            _apfs[i] = new AllPassFilter(freq, apf_g);
            _nApfs++;
        }
    }

    void rt60(float rt60) {
        _rt60 = rt60;
        _dirty = true;
    }

    void apfG(float g) {
        _apfG = g;
        _dirty = true;
    }

    void dryWetMix(float mix) {
        _dryWetMix = mix;
    }


    float process(float input) {
        // is there anything that we need to do up front?
        if(_dirty) {
            cookParams();
        }

        // run the comb filters
        float combOut = 0;
        for(int i=0; i < _nCombs; i++) {
            combOut += _combs[i]->process(input);
        }

        // run the all-pass filters
        float apfAccum = combOut;
        for(int i=0; i < _nApfs; i++) {
            apfAccum = _apfs[i]->process(apfAccum);
        }

        // I don't have this totally nailed down...
        float wetOut = apfAccum * _wetGain;

        // mix it up
        float out = ((1-_dryWetMix) * input) + (_dryWetMix * wetOut);

        return out;
    }

private:
    void cookParams() {
        for(int i=0; i < _nCombs; i++) {
            auto cf = _combs[i];
            float comb_g = combG(cf->delay());
            cf->g(comb_g);
        }

        for(int i=0; i < _nApfs; i++) {
            _apfs[i]->g(_apfG);
        }

        _dirty = false;
    }

	float combG(size_t delay) {
		return pow(10.f, (-3.f*delay/_sampleRate)/_rt60);
	}
};



#endif