#ifndef WAV_OSC_HPP
#define WAV_OSC_HPP

#define DR_WAV_IMPLEMENTATION
#include "../../dep/dr_wav.h"

#include "constants.hpp"

/**
 * @brief Base class for oscillators that operate off of a phase cycle of 2π.
 * 
 * WavOscBase is meant to be a starting point that has the core plumbing taken care
 * of so that different waveforms or waveform generation methods can be
 * explored.
 */
class WavOscBase {    
protected:
    float* _waveTable = NULL;
    int _cycleLength = 1;
    int _sampleRate;
    float _freq = 0.1;
    float _phase = 0.f;
    float _phaseInc = 0.1f;
    int _dirty = true;


public:
    WavOscBase(int sampleRate) : _sampleRate(sampleRate) {}

    ~WavOscBase() {
        if(_waveTable != NULL)
            drwav_free(_waveTable, NULL);
    }

    void loadWaveTable(const char* filename, float start=0) {
        if( _waveTable != NULL) {
            drwav_free(_waveTable, NULL);
        }

        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 numSamples;
        _waveTable = drwav_open_file_and_read_pcm_frames_f32(filename, &channels, &sampleRate, &numSamples, NULL);
        if (_waveTable == NULL) {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }
        _cycleLength = numSamples;
        _phase = start;
        _dirty = true;
    }

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

    float nextSample(int interpolate=true) {
        float out = interpolate ? _getSampleInterpolated(_phase) : _getSample(round(_phase));
        _cycle();
        return out;
    }
     
protected:
    void _calcPhaseInc() {
        _phaseInc = _cycleLength * _freq / _sampleRate;
        _initCycle();
        _dirty = false;
    }
    
    void _cycle() {
        if(_dirty)
            _calcPhaseInc();

        _phase += _phaseInc;
        if(_phase > _cycleLength)
            _phase -= _cycleLength;
    }
    
    float _getSampleInterpolated(float idx) {
        int phaseInt = floor(_phase);
        float phaseFraction = _phase - phaseInt;
        float s1 = _getSample(phaseInt);
        float s2 = _getSample(phaseInt+1);
        return s1 + (phaseFraction * (s2 - s1));
    }

    float _getSample(int idx) {
        if(idx >= _cycleLength)
            idx = idx % _cycleLength;
        return _waveTable[idx];
    }

    virtual void _initCycle() { ; }
};


#endif