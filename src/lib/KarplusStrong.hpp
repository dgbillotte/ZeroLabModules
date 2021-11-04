#ifndef KARPLUS_STRONG_HPP
#define KARPLUS_STRONG_HPP

#include <chrono>
using namespace std::chrono;
#include <thread>
#include "../../dep/dr_wav.h"
#include "DelayBuffer.hpp"
#include "Filter.hpp"
#include "Util.hpp"
#include "WavFile.hpp"
#include "WavFileStore.hpp"


/*
 * To do/explore:
 * - experiment with different impulse wav-files
 * - add noise to wav impulses
 * - mix several different waveforms/noise
 * - hammer-on/off
 * - allow for user defined impulses either as a file or a buffer of samples
 * - other impulse or feedback filters
 */
class KarplusStrong {
    // parameters
    int _sampleRate;
    float _p = 1.f; // decay factor in 0..1
    float _S = 0.5f; // decay "stretching" factor in 0..1. 0.5 produces most minimal decay
    int _impulseFiltersOn = true;
    int _impulsePickPosOn = true;
    float _pickPos = 0.f;
    int _impulseDynamicsOn = true;
    float _dynamicLevel = 10.f;  // dynamics gain level
    float _lpfFreq = 5000.f;

    // calculated parameters
    // float _R = 0.f; // calculated value for R for the dynamics filter
    float _Pc = 0.99f; // calculated value for the phase coefficient for tuning correction

    // impulse thread
    std::thread _impulseThread;
    int _keepWorking = true;
    // this communicates both the existence of a job to do and which wave type.
    // -1 indicates no work to do
    int _impulseType = -1; 

    // book-keeping
    size_t _delayLength = 1;
    size_t _write_i = 1000000;
    int _attack_on = 0;

    // delays and filters
    DelayBuffer<float> _delayLine;
    DelayBuffer<float> _impulseDelay;
    TwoPoleBPF _impulseBPF;
    OnePoleLPF _impulseLPF;

    // static members
    static int __numInstances;
    static WavFileStore __wavefiles;

    bool _haveExternalOTFSample = false;
    float _externalOTFSample = 0.f;

    float* _externalImpulseWavetable = nullptr;
    size_t _externalImpulseNumSamples = 0;

public:
    enum ImpulseTypes {
        EXTERNAL_OTF,
        WHITE_NOISE,
        // PINK_NOISE,
        // BROWN_NOISE,
        // SINE40,
        SINE100,
        SINE500,
        SINE1000,
        // SINE256,
        // SINE_CHIRP,
        // SQUARE_CHIRP,
        RANDOM_SQUARE,
        NOISE_OTF,      
        NUM_IMPULSE_TYPES        
    };

    KarplusStrong(int sampleRate, int maxDelay=5000);
    ~KarplusStrong();
    
    // parameter setters -----------------------------------------------------------------
    void sampleRate(int sampleRate) { _sampleRate = sampleRate; }
    void p(float p) { _p = p; } // Decay needs to be |p| <= 1. todo: try out neg values
    void S(float S) { _S = S; } // Stretch needs to be 0 < S < 1
    void impulseFiltersOn(int apply) { _impulseFiltersOn = apply; }
    void pickPosOn(int isOn) { _impulsePickPosOn = isOn; }    
    void pickPos(float pos) { _pickPos = pos; } // pickPos should be 0 <= pickPos <= 1
    void dynamicsOn(int isOn) { _impulseDynamicsOn = isOn; }
    void dynamicsLevel(float level) { _dynamicLevel = level; } //This should be greater than 1
    void lpfFreq(float freq) { _lpfFreq = freq; } //This should be greater than 1


    void pluck(float freq, float attackLength=1.f, int impulseType=0);
    void refret(float freq);
    float nextValue();

    // void hammerOn(float freq);
    // void hammerOff();

    void setOTFSample(float sample);
    void setImpulseWavetable(float* wt, size_t numSamples);


protected:
    // keep writing the impulse util it is _delayLength long
    // countdown the _attack
    void _excite_old() {

        // this is where OTF impulse generation happens
        if(_write_i < _delayLength) {
            float sample = _haveExternalOTFSample ? _externalOTFSample : _randf01();
            if(_impulseFiltersOn) {
                sample = _impulseFilters(sample);
            }
            _haveExternalOTFSample = false;
            _delayLine.push(sample);
            _write_i++;
        }

        if(_attack_on > 0) {
            _attack_on--; // this should NOT go negative
        }
    }
    float _excite() {

        float x0 = _delayLine.read();

        // this is where OTF impulse generation happens
        if(_write_i < _delayLength) {
            float sample = _haveExternalOTFSample ? _externalOTFSample : _randf01();
            if(_impulseFiltersOn) {
                sample = _impulseFilters(sample);
            }
            _haveExternalOTFSample = false;
            _delayLine.push(sample);
            _write_i++;
        }

        // generate the attack 
        if(_attack_on > 0) {
            _attack_on--; // this should NOT go negative

            if(_attack_on > 0) {
                // float y0 = x0; //_impulseFilters(x0);
                _delayLine.write(-1, x0);

                // if on-the-fly is not running
                if(_write_i == _delayLength) {
                    _delayLine.push(x0);
                }
                // return y0;
            }            
        }
        return x0;
    }


    // do all the heavy calculations needed on a freq change
    void _setFreqParams(float freq) {
        // float Pa = 0.5f; // use this if not using stretch (_S)
        float wT = 2.f*M_PI*freq / _sampleRate;
        float Pa = -atan((-_S*sin(wT))/((1.f - _S) + _S * cos(wT))) / wT;
        float P1 = _sampleRate/freq;
        _delayLength = P1 - Pa - 0.00001f;
        _delayLine.size(_delayLength);
        _Pc = P1 - _delayLength - Pa;
    }


    void _startImpulse(float freq, float attackLength, int type) {
        // configure impulse filters
        //_computeR(freq);
        _impulseDelay.size(_delayLength * _pickPos);
        _impulseBPF.freq(freq);
        _impulseBPF.q(_dynamicLevel);
        _impulseLPF.freq(_lpfFreq);

        // load the impulse and start it running by setting _attack_on > 0
        _loadImpulse(type);
        _attack_on = _delayLength * attackLength;
    }

    int _otfImpulseType = 0;
    void _loadImpulse(int type, float gain=1.f) {
        _delayLine.clear();
        _impulseDelay.clear();
        _write_i = _delayLength; // turn on-the-fly off

        if(type == EXTERNAL_OTF) {
            _write_i = 0;

        } else if(type < RANDOM_SQUARE) {
            // all of the wav-file based impulses
            _startImpulseJob(type);


        } else if(type == RANDOM_SQUARE) {
            // the generated random-square impulse
            float sample;
            for(size_t i=0; i < _delayLength; i++) {
                sample = _randf01() >= 0 ? 1.f : -1.f;
                _delayLine.push(sample * gain);
            }

        } else { //if(type == NOISE_OTF) {
            // this will initiate the on-the-fly impulse generation
            _write_i = 0; 
        }
    }

    float _randf01() {
        return 2.f * rand() / (float)RAND_MAX - 1.f;
    }



    void _fillDelay(float* source, size_t len) {
        float* delayBuf = _delayLine.getBuffer();
        if(len >= _delayLength) {
            memcpy(delayBuf, source, _delayLength * sizeof(float));
        } else {
            size_t i = 0;
            for(; i < (_delayLength / len); i++) {
                memcpy(&(delayBuf[i*len]), source, len * sizeof(float));
            }
            memcpy(&(delayBuf[i*len]), source, (_delayLength - (i*len)) * sizeof(float));
        }        
        _delayLine.resetHead();
    }

    // this is takes too long to run in the audio loop.
    void _fillDelayFiltered(float* source, size_t len) {
        float* delayBuf = _delayLine.getBuffer();
        if(len >= _delayLength) {
            for(size_t i=0; i < len; i++) {
                delayBuf[i] = _impulseFilters(source[i]);
            }

        } else {
            for(size_t count=0; count < _delayLength;) {
                for(size_t i=0; i < len; i++) {
                    if(count++ == _delayLength) {
                        break;
                    }
                    delayBuf[(count*len) + i] = _impulseFilters(source[i]);
                }
            }
        }
        _delayLine.resetHead();
    }

    float _impulseFilters(float input) {
        float out = input;
        if(_impulsePickPosOn) {
            out -= _impulseDelay.read();
            if(out > 1.f) {
                out = 1.f;
            } else if(out < -1.f) {
                out = -1.f;
            }
            _impulseDelay.push(out);
        }

        if(_impulseDynamicsOn) {
            // this doesn't simulate proper dynamics w R, but is interesting
            out = _impulseLPF.process(out);
        }
        return out;
    }
    
  
    // -----------------------------------------------------------------------------------
    // --------------------- Impulse Thread Functions ------------------------------------

    // this should get called in the ctor
    void _startImpulseThread() {
        _impulseThread  = std::thread(&KarplusStrong::_impulseWorker, this);
    }

    // call this in the destructor
    void _killImpulseThread() {
        _keepWorking = false;
    }


    // call this each time there is a new impulse to process
    void _startImpulseJob(int type) {
        _impulseType = type;
    }

    // this is the body of the long running worker thread
    // thread-safety note: we should ensure that there is
    // no overlap in jobs running or if there is it is 
    // coordinated. I can see this happening mainly if
    // it gets plucked many times quickly...
    void _impulseWorker() {
        while(_keepWorking) {

            if(_impulseType == EXTERNAL_OTF) {
                if(_impulseFiltersOn ) {
                    _fillDelayFiltered(_externalImpulseWavetable, _externalImpulseNumSamples);

                } else {
                    _fillDelay(_externalImpulseWavetable, _externalImpulseNumSamples);
                }
                _impulseType = -1;

            } else if(_impulseType >= 0) {
                WavFilePtr wf = __loadImpulseFile(_impulseType);

                if(_impulseFiltersOn ) {
                    _fillDelayFiltered(wf->waveTable(), wf->numSamples());

                } else {
                    _fillDelay(wf->waveTable(), wf->numSamples());
                }
                _impulseType = -1;
                
            } else {
                std::this_thread::yield();
            }
        }
    }

    // --------------------- End of Impulse Thread Functions -----------------------------
    // -----------------------------------------------------------------------------------
    static WavFilePtr __loadImpulseFile(int fileNum);



    
};

#endif