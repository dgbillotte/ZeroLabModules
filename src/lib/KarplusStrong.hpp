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
    bool _impulseFiltersOn = true;
    bool _impulsePickPosOn = true;
    float _pickPos = 0.f;
    bool _impulseLpfOn = true;
    float _lpfFreq = 5000.f;

    // calculated parameters
    // float _R = 0.f; // calculated value for R for the dynamics filter
    float _Pc = 0.99f; // calculated value for the phase coefficient for tuning correction

    // impulse thread
    std::thread _impulseThread;
    bool _keepWorking = true; // communicate to worker thread when time to shut down
    int _impulseType = -1;  // -1 indicates no work to do, other values are wave types to load

    // book-keeping
    float _currentFreq;
    size_t _delayLength = 1;
    size_t _write_i = 1000000;
    int _attack_on = 0;

    // old values for hammer on/off
    float _lastFreq = 0.f;
    size_t _lastDelayLength = 0;
    float _lastPc = 0.f;

    // delays and filters
    DelayBuffer<float> _delayLine;
    DelayBuffer<float> _impulseDelay;
    OnePoleLPF _impulseLPF;

    // external impulse handling
    bool _haveExternalOTFSample = false;
    float _externalOTFSample = 0.f;
    float* _externalImpulseWavetable = nullptr;
    size_t _externalImpulseNumSamples = 0;

    // static members
    static int __numInstances;
    static WavFileStore __wavefiles;


public:
    enum ImpulseTypes {
        EXTERNAL_BUFFER,
        EXTERNAL_OTF,
        NOISE_OTF,
        WHITE_NOISE,
        SINE100,
        SINE500,
        SINE1000,
        RANDOM_SQUARE,
        NUM_IMPULSE_TYPES        
    };

    KarplusStrong(int sampleRate, int maxDelay=5000);
    ~KarplusStrong();
    
    // --------------------- Parameter Setters ------------------------------------
    /* ----------------------------------------------------------------------------
     * - sampleRate: int, > 0
     * - p (decay): float, |p| <= 1, todo: try out neg values
     * - S (stretch, opposite of decay, kinda): float, 0 < S < 1
     * - impulse filters:
     *   - all impulse filters on/off
     *   - pick position on/off
     *   - pickPos: float, 0 <= pickPos <= 1. 0..0.5 seems most useful
     *   - lpfOn on/off
     *   - lpfFreq: float, 20? < freq < 20k?
     *   - dynamics stuff: get rid of
     */
    void sampleRate(int sampleRate) { _sampleRate = sampleRate; }
    void p(float p) { _p = p; }
    void S(float S) { _S = S; }
    void impulseFiltersOn(int apply) { _impulseFiltersOn = apply; }
    void pickPosOn(bool isOn) { _impulsePickPosOn = isOn; }
    void pickPos(float pos) { _pickPos = pos; }
    void impulseLpfOn(bool isOn) { _impulseLpfOn = isOn; }
    void impulseLpfFreq(float freq) { _lpfFreq = freq; }


    // --------------------- Make it make noise -----------------------------------
    /* ---------------------------------------------------------------------------- */
    void pluck(float freq, float attackLength=1.f, int impulseType=0);
    void refret(float freq);
    float nextValue();

    void hammerOn(float freq);
    void hammerOff();

    // --------------------- Set the impulse sources ------------------------------
    /* ---------------------------------------------------------------------------- */
    void setOTFSample(float sample);
    void setImpulseWavetable(float* wt, size_t numSamples);


protected:
    // generate the attack of a pluck
    float _excite();

    // do all the heavy calculations needed on a freq change
    void _setFreqParams(float freq);

    void _startImpulse(float freq, float attackLength, int type);

    void _loadImpulse(int type, float gain=1.f);

    float _randf01();

    // fill the delay from a float* source
    void _fillDelay(float* source, size_t len);
    void _fillDelayMemcpy(float* source, size_t len);
    void _fillDelayFiltered(float* source, size_t len);

    // run any and all of the active impulse filters on a single sample
    float _impulseFilters(float input);
    
  
    // -----------------------------------------------------------------------------------
    // --------------------- Impulse Thread Functions ------------------------------------

    // this should get called in the ctor
    void _startImpulseThread();

    // call this in the destructor
    void _killImpulseThread();

    // call this each time there is a new impulse to process
    void _startImpulseJob(int type);

    // this is the body of the long running worker thread
    // thread-safety note: we should ensure that there is
    // no overlap in jobs running or if there is it is 
    // coordinated. I can see this happening mainly if
    // it gets plucked many times quickly...
    void _impulseWorker();

    // STATIC method for loading wavefiles from disk (if they aren't loaded)
    static WavFilePtr __loadImpulseFile(int fileNum);
    
};

#endif