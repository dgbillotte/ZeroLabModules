/*
Schroeder Reverb:
This is an implementation of the Schroeder Reverb as described by Will Pirkle.

NOTE: Currently, when you reopen a patch with this module in it, the module
exhibits not-very predictable behavior on start up and commonly locks into
a super-saturated state. It will usually calm down if you unplug the input,
turn all the knobs to zer0, and give it a second for the delays to
clear out. I might get this fixed...

It is composed of M parallel comb filters that are summed together and then
passed through N all-pass filters serially. In the book, M=4 and N=2 but this
implementation allows the user to determine M & N. Below is a 4x2 instance.

The output gain on the above combo is pretty high (10x in some cases) so I've
played with various means of taming it. Currently I just attenuate it by _wetGain
right before mixing it to out.

int combf_freqs[] =  {1321, 1447, 1657, 1993};
int apf_freqs[] = {33, 89};
ReverbS reverb = ReverbS(combf_freqs, apf_freqs);

The comb filter delays are based on Schroeder's suggestion
of 1:1.5 ratio for the delays of the comb filters, specifically
30-45msec. I calc'd this based on 44100 and then split the range
up two ways. First was evenly, but I rounded to the nearest prime:
D0: 1321, 1543, 1777, 1993
The second is based on the golden ratio, again rounding to nearest prime
D1: 1321, 1447, 1657, 1993


For the all-pass filter, again based on Schroeder's suggestion, 
of 1-5msec. This is the list of primes to choose from:
[43, 47, 53, 61, 71, 73, 79, 83, *89, 97, 101, 103, 107, 109, 113,
 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191,
 193, 197, 199, 211, 223, 227, 229, *233]

Some things to do...
- currently the delays are in samples. Change to or add time based (sec or ms) spec.
- figure out the startup lockup
- make it possible to externally configure the delay times for
  the filters and possibly the number of each
- do log mixing instead of linear mixing?

*/
#ifndef REVERB
#define REVERB

#include "AllPassFilter.hpp"
#include "CombFilter.hpp"

class ReverbS {
    CombFilter cf1 = CombFilter(1321, combG(1321));
    CombFilter cf2 = CombFilter(1543, combG(1543));
    CombFilter cf3 = CombFilter(1777, combG(1777));
    CombFilter cf4 = CombFilter(1993, combG(1993));

    AllPassFilter apf1 = AllPassFilter(33, 0.5f);
    AllPassFilter apf2 = AllPassFilter(89, 0.5f);

    float _rt60 = 0.5f;
    float _apfG = 0.5f;
    int _sampleRate = 44100;
    float _dryWetMix = 0.5f;
    int _dirty = false;
    const float _wetGain = 0.7f;



public:
    // ReverbS(int sampleRate=44100, float rt60=0.5f, float apfG=0.5f, dryWetMix=0.5f) :
    //     _rt60(rt60), _apfG(apfG), _sampleRate(sampleRate), _dryWetMix(dryWetMix) {}
    ReverbS(int sampleRate=44100) : _sampleRate(sampleRate) {}

    void rt60(float rt60) {
        if(_rt60 == rt60)
            return;
        _rt60 = rt60;
        _dirty = true;
    }

    void apfG(float g) {
        if(_apfG == g)
            return;
        _apfG = g;
        _dirty = true;
    }

    void dryWetMix(float mix) {
        _dryWetMix = mix;
    }

    void sampleRate(int sampleRate) {
        if(_sampleRate == sampleRate)
            return;
        _sampleRate = sampleRate;
        _dirty = true;
    }

    float process(float input) {
        if(_dirty) {
            cf1.g(combG(cf1.delay()));
            cf2.g(combG(cf2.delay()));
            cf3.g(combG(cf3.delay()));
            cf4.g(combG(cf4.delay()));

            apf1.g(_apfG);
            apf2.g(_apfG);

            _dirty = false;
        }

        // run the comb filters
        float out = cf1.process(input) + cf2.process(input) +
            cf3.process(input) + cf4.process(input);

        // run the all-pass filters            
        float wetOut = _wetGain * apf2.process(apf1.process(out));

        // mix it up, this should prob be log instead of linear
        out = ((1-_dryWetMix) * input) + (_dryWetMix * wetOut);

        return out;
    }

private:
	float combG(size_t delay) {
		return pow(10.f, (-3.f*delay/_sampleRate)/_rt60);
	}
};


#endif