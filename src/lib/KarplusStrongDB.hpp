#ifndef KARPLUS_STRONG_DB_HPP
#define KARPLUS_STRONG_DB_HPP

#include "DelayBuffer.hpp"
#include "Filter.hpp"
#include "AllPassFilter.hpp"

#include "../../dep/dr_wav.h"

const char* filename = "/Users/daniel/projects/rack-plugins/ZeroLab/res/sqr-chirp-5k-samples.wav";

/*
 * This is my first, naive attempt. I did not fully understand the algorithm yet...
 */

class KarplusStrongDB {
    int _sampleRate;
    int _delayLength = 1;
    DelayBuffer<float> _delayLine;
    SimpleOnePoleLPF _lpf;
    dsp::SchmittTrigger _trigger;
    float* _wavetable = NULL;

public:
    KarplusStrongDB(int sampleRate, int maxDelay=5000) :
        _sampleRate(sampleRate),
        _delayLine(maxDelay),
        _lpf(440, sampleRate)
    {}

    ~KarplusStrongDB() {
        if(_wavetable)
            drwav_free(_wavetable, NULL);
    }

    void sampleRate(int sampleRate) {
        _sampleRate = sampleRate;
        _lpf.sampleRate(_sampleRate);
    }
    void lpfFreq(float freq) {
        _lpf.freq(freq);
    }

    float pluck(float freq, int useWavetable=false) {
        _delayLength = round(_sampleRate/freq);
        std::cout << "freq: " << freq << " delay: " << _delayLength <<
            " sr: " << _sampleRate << std::endl;
        _excite(useWavetable);

        return nextValue();
    }

    float nextValue() {
        float y = _delayLine.read(_delayLength);
        float x = _delayLine.read(0);
        float out = (x+y)/2;
        out = _lpf.process(out);
        _delayLine.push(out);

        return out;
    }

    float currentValue() {
        return _delayLine.read(0);
    }

protected:
    const float SPEED_OF_SOUND = 1125.f;
    int _sizeToDelay(float lengthFeet) {
        float secs = lengthFeet/SPEED_OF_SOUND;
        return secs * _sampleRate;
    }

    void _excite(int useWavetable=false) {
        useWavetable = 0;
        if(useWavetable == 1) {
            for(int i=0; i < _delayLength; i++) {
                _delayLine.push((_wavetable[i]));
            }
            
        } else if (useWavetable == 2) {
            for(int i=0; i < _delayLength; i++) {
                float s = ((rand() / (float)RAND_MAX) *2.f - 1.f) * 5.f;
                _delayLine.push((_wavetable[i]+s)/2);
            }
        
        } else {
            for(int i=0; i < _delayLength; i++) {
                float s = ((rand() / (float)RAND_MAX) *2.f - 1.f) * 5.f;
                _delayLine.push(s);
            }        
        }
    }

    void _loadImpuleFile() {
        // load an impulse file
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 numSamples;

        _wavetable = drwav_open_file_and_read_pcm_frames_f32(filename, &channels, &sampleRate, &numSamples, NULL);
        if (_wavetable == NULL) {
        	std::cerr << "Unable to open file: " << filename << std::endl;
        }
    }
};

#endif