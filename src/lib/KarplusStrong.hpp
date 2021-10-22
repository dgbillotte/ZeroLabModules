#ifndef KARPLUS_STRONG_HPP
#define KARPLUS_STRONG_HPP

#include <chrono>
using namespace std::chrono;
#include "../../dep/dr_wav.h"
#include "DelayBuffer.hpp"
#include "Filter.hpp"
#include "Util.hpp"

// uncomment below to include timing logging for impulse loading
// #define TIME_IMPULSE_LOAD
// #define LOG_IMPULSE_STATS

struct WaveFile {
    const char* filename;
    float* wavetable;
    int numSamples;
    // float gainTo1 = 1.f;


    // WaveFile(const char* filename, float gainTo1=1.f) : filename(filename), gainTo1(gainTo1) {}
    WaveFile(const char* filename) {
        this->filename = filename;
    }
};



class KarplusStrong {
    int _sampleRate;
    size_t _delayLength = 1;
    DelayBuffer<float> _delayLine;
    DelayBuffer<float> _impulseDelay;
    float _Pc = 0.99f; // calculated value for the phase coefficient for tuning correction
    float _p = 1.f; // decay factor in 0..1
    float _S = 0.5f; // decay "stretching" factor in 0..1. 0.5 produces most minimal decay
    float _dynamicLevel = 10.f;  // dynamics gain level
    // float _R = 0.f; // calculated value for R for the dynamics filter
    size_t _write_i = 1000000;
    int _attack_on = 0;
    float _pickPos = 0.f;
    TwoPoleBPF _impulseBPF;
    static int __numInstances;

public:
    enum ImpulseTypes {
        WHITE_NOISE,          
        PINK_NOISE,          
        BROWN_NOISE,          
        SINE40,
        SINE100,
        SINE500,
        SINE1000,
        SINE256,
        SINE_CHIRP,
        // SQUARE_CHIRP,
        RANDOM_SQUARE,
        NOISE_OTF,      
        NUM_IMPULSE_TYPES        
    };

    KarplusStrong(int sampleRate, int maxDelay=5000) :
        _sampleRate(sampleRate),
        _delayLine(maxDelay),
        _impulseDelay(maxDelay),
        _impulseBPF(sampleRate, 440.f, 0.01f)
    {
        _delayLine.clear();
        _impulseDelay.clear();
        __numInstances++;
    }

    ~KarplusStrong();


    // Decay needs to be |p| <= 1. todo: try out neg values
    // Stretch needs to be 0 < S < 1
    // pickPos should be 0 <= pickPos <= 1

    void sampleRate(int sampleRate) { _sampleRate = sampleRate; }
    void p(float p) { _p = p; }
    void S(float S) { _S = S; }
    void dynamicsLevel(float level) { _dynamicLevel = level; } //This should be greater than 1
    void pickPos(float pos) { _pickPos = pos; }

    // for now, attackLength is a multiplier, not actual length.
    // In should be in [0,5+], neg values not good
    void pluck(float freq, float attackLength=1.f, int impulseType=0) {
        // calculate the _delayLength and _Pc parameter
        _setFreqParams(freq);
        _startImpulse(freq, attackLength, impulseType);
        attackRunning = true;
     }

    void refret(float freq) {
        _setFreqParams(freq);
    }

    
#ifdef LOG_IMPULSE_STATS
    StatsF impulseStats;
#endif

    const float gain = 10.f;
    int attackRunning = true;
    float nextValue(int log=false) {
        // run the exciter
        _excite();

        // get the "input"
        float x0 = _delayLine.read();

        // if attack is on, run the impulse filters and store the value back into the delay
        if(_attack_on > 0) {
            // apply impulse filters on the fly
            // note: for attack > 1 these will get applied mult times. might need to remedy...
            float y0 = x0; //_impulseFilters(x0);
            _delayLine.write(-1, y0);

            // calculate impulse statistics
#ifdef LOG_IMPULSE_STATS
            impulseStats.sample(x0);
#endif

            // if on-the-fly is not running
            if(_write_i == _delayLength) {
                _delayLine.push(y0);
            }
            return y0 * gain;
        }

        // this was designed for logging info about the impulse, but could be used for other purposes...
        if(attackRunning == true) {
            attackRunning = false;

#ifdef LOG_IMPULSE_STATS
            // do some logging or other stuff...
            std::cout << "delayLength: " << _delayLength << ", attack_on: " << _attack_on << std::endl;
            std::cout << "samples: " << impulseStats.count << ", mean: " << impulseStats.mean() <<
                ", min: " << impulseStats.min << ", max: " << impulseStats.max << std::endl << std::endl;
#endif
        }

        // read last two items from the delay
        float y0 = _delayLine.read(-1);
        float y1 = _delayLine.read(-2);

        // this is the standard KP with a 2-point average
        // float out = (x + (y0 + y1)/2)/2; // the 2nd /2 isn't mentioned, but it blows up without it....

        // KP with a 2-point weighted average
        float out = (x0 + _p*((1-_S)*y0 + _S*y1))/2; // the 2nd /2 isn't mentioned, but it blows up without it....
        _delayLine.push(out);

        // all-pass filter to correct tuning
        float Pc = 0.5f;
        float C = (1 - Pc) / (1 + Pc);
        out = C * out + x0 - C * y0;

        return out * gain;
    }

    float currentValue() {
        return _delayLine.read(0);
    }

protected:
    // keep writing the impulse util it is _delayLength long
    // countdown the _attack
    void _excite() {
        if(_write_i < _delayLength) {
            _delayLine.push(_randf01());
            _write_i++;
        }

        if(_attack_on > 0) {
            _attack_on--; // this should NOT go negative
        }
    }

    float _impulseFilters(float input) {
        float out = _impulseBPF.process(input);// - _impulseDelay.read();
        // _impulseDelay.push(input);
        return out;
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
#ifdef TIME_IMPULSE_LOAD
    int numTests = 0;
    int elapsed_us = 0;
#endif
    void _startImpulse(float freq, float attackLength, int type) {
        // impulse filters
        //_computeR(freq);
        _impulseDelay.size(_delayLength * _pickPos);
        _impulseBPF.freq(freq);
        _impulseBPF.q(_dynamicLevel);

#ifdef TIME_IMPULSE_LOAD
        auto start = high_resolution_clock::now();
#endif
        _loadImpulse(type);
#ifdef TIME_IMPULSE_LOAD
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop-start);
        int thisRun = duration.count();
        elapsed_us += thisRun;
        numTests++;
        std::cout << "Impulse Load took: " << thisRun << "us. Average: " <<
            elapsed_us/numTests << "us." << std::endl;
#endif
        _attack_on = _delayLength * attackLength;
    }

    float _randf01() {
        return 2.f * rand() / (float)RAND_MAX - 1.f;
    }

    void _loadImpulse(int type, float gain=1.f) {
        _delayLine.clear();
        _impulseDelay.clear();
        _write_i = _delayLength; // turn on-the-fly off

        if(type < RANDOM_SQUARE) {
            WaveFile wf = _loadImpulseFile(type);
            // _delayLine.fillBuffer(wf.wavetable, wf.numSamples);
            fillDelayFiltered(wf.wavetable, wf.numSamples);

        } else if(type == RANDOM_SQUARE) {
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

    void fillDelayFiltered(float* source, size_t len) {
        float* delayBuf = _delayLine.getBuffer();
        if(len >= _delayLength) {
            for(size_t i=0; i < len; i++) {
                delayBuf[i] = _impulseFilters(source[i]);
            }
        } else {

            // int count = 0;
            for(int count=0; count < _delayLength;) {
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

    void fillDelay(float* source, size_t len) {
        float* delayBuf = _delayLine.getBuffer();
        if(len >= _delayLength) {
            memcpy(delayBuf, source, _delayLength * sizeof(float));
        } else {
            int i = 0, numCopies = _delayLength / len;
            for(; i < numCopies; i++) {
                memcpy(&(delayBuf[i*len]), source, len * sizeof(float));
            }
            memcpy(&(delayBuf[i*len]), source, (_delayLength - (i*len)) * sizeof(float));
        }        
        _delayLine.resetHead();
    }

    WaveFile& _loadImpulseFile(int fileNum);
  

    /*
     * This isn't working as of now. It has made some noise, but then cuts out.
     * I'm not sure about some of the places where I've used _freq and f1 below.
     * In the text "f" is used only once and "f1" is used in an ambiguous way and
     * I'm not sure if the lines for tmp and pift are correct
     * 
     * todo:
     * - analyze the values coming out to see where it is falling off
     * - create simpler approximation of this value. currently it has:
     *   - 7 expensive func calls: exp, sqrt, cos, sin
     *   - 21 mulitiply/divides
     */
    // void _computeR(float freq) {
    //     float f1 = 20.f;
    //     float fu = 10000.f;
    //     float fm = sqrt(f1*fu);
    //     float Rl = exp(-M_PI * _dynamicLevel / _sampleRate);
    //     float tmp = (-freq * 2.f * M_PI * fm) / _sampleRate;
    //     float Gl = (1.f - Rl) / (1.f - Rl*exp(tmp));
    //     float G2 = Gl*Gl;
    //     float pift = M_PI * f1 / _sampleRate;
    //     float p1 = (1.f - G2 * cos(2.f * pift)) / (1.f - G2);
    //     float cospift = cos(pift);
    //     float p2 = (2.f * Gl * sin(pift) * sqrt(1.f - (G2 * cospift * cospift))) / (1.f - G2);
    //     float r1 = p1 + p2;
    //     _R = (r1 < 1) ? r1 : (p1 - p2);
    // }
    
};

#endif