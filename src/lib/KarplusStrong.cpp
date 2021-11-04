#include "../plugin.hpp"

#include "KarplusStrong.hpp"
#include "WavFileStore.hpp"


// --------------------- Static Members ---------------------------------------
// ----------------------------------------------------------------------------
int KarplusStrong::__numInstances = 0;


const char* files[5] = {
    "res/white-noise-1000-samples.wav",
    "res/sine100.wav",
    "res/sine500.wav",
    "res/sine1000.wav",
    "res/sine_256.wav"
};


WavFileStore KarplusStrong::__wavefiles(files, 5);

// --------------------- Constructor/Destructor -------------------------------
// ----------------------------------------------------------------------------
KarplusStrong::KarplusStrong(int sampleRate, int maxDelay) :
    _sampleRate(sampleRate),
    _delayLine(maxDelay),
    _impulseDelay(maxDelay),
    _impulseBPF(440.f, sampleRate),
    _impulseLPF(5000.f, sampleRate)
{
    __numInstances++;
    _delayLine.clear();
    _impulseDelay.clear();
    _startImpulseThread();
}


KarplusStrong::~KarplusStrong() {
    _killImpulseThread();
    while(! _impulseThread.joinable()) {
        std::this_thread::yield();
    }
    _impulseThread.join();

    if(--__numInstances == 0) {
        __wavefiles.clearCache();
    }
}

// --------------------- External Impulse Injection ---------------------------
// ----------------------------------------------------------------------------
void KarplusStrong::setOTFSample(float sample) {
    _externalOTFSample = sample;
    _haveExternalOTFSample = true;
}

void KarplusStrong::setImpulseWavetable(float* wt, size_t numSamples) {
    _externalImpulseWavetable = wt;
    _externalImpulseNumSamples = numSamples;
}


// --------------------- Public Methods ---------------------------------------
// ----------------------------------------------------------------------------

void KarplusStrong::pluck(float freq, float attackLength, int impulseType) {
    _setFreqParams(freq);
    _startImpulse(freq, attackLength, impulseType);
}

void KarplusStrong::refret(float freq) {
    _setFreqParams(freq);
}

// void hammerOn(float freq) {
//     _savedFreq = currentFreq;
//     refret(freq);
// }
// void hammerOff() {
//     refret(_savedFreq)
// }

float KarplusStrong::nextValue() {
    // do any excitation and get the x0 value from the delay-line
    float x0 = _excite();
    if(_attack_on > 0) {
        return x0;
    }

    // read last two items from the delay
    float y0 = _delayLine.read(-1);
    float y1 = _delayLine.read(-2);

    // KP with a 2-point weighted average
    float out = (x0 + _p*((1-_S)*y0 + _S*y1))/2; // the 2nd /2 isn't mentioned, but it blows up without it....
    _delayLine.push(out);

    // all-pass filter to correct tuning
    // TODO: !!!!! we aren't invoking the tuning filter!!!!@
    float Pc = 0.5f;
    float C = (1 - Pc) / (1 + Pc);
    out = C * out + x0 - C * y0;

    return out;
}


// --------------------- Private Methods --------------------------------------
// ----------------------------------------------------------------------------

// keep writing the impulse util it is _delayLength long
// countdown the _attack
float KarplusStrong::_excite() {

    float x0 = _delayLine.read();

    // this is where OTF impulse generation happens
    // keep writing the impulse until it is _delayLength long
    if(_write_i < _delayLength) {
        float sample = _haveExternalOTFSample ? _externalOTFSample : _randf01();
        if(_impulseFiltersOn) {
            sample = _impulseFilters(sample);
        }
        _haveExternalOTFSample = false;
        _delayLine.push(sample);
        _write_i++;
    }

    // countdown the attack and pump values through the delay-line as needed
    if(_attack_on > 0) {
        _attack_on--; // this should NOT go negative

        // if on-the-fly is not running pump the value back into the delay
        if(_write_i == _delayLength) {
            _delayLine.push(x0);
        }         
    }
    return x0;
}

void KarplusStrong::_setFreqParams(float freq) {
    // float Pa = 0.5f; // use this if not using stretch (_S)
    float wT = 2.f*M_PI*freq / _sampleRate;
    float Pa = -atan((-_S*sin(wT))/((1.f - _S) + _S * cos(wT))) / wT;
    float P1 = _sampleRate/freq;
    _delayLength = P1 - Pa - 0.00001f;
    _delayLine.size(_delayLength);
    _Pc = P1 - _delayLength - Pa;
}


void KarplusStrong::_startImpulse(float freq, float attackLength, int type) {
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

void KarplusStrong::_loadImpulse(int type, float gain) {
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

float KarplusStrong::_randf01() {
    return 2.f * rand() / (float)RAND_MAX - 1.f;
}
// --------------------- Fill the Delay Related -------------------------------
// ----------------------------------------------------------------------------

void KarplusStrong::_fillDelay(float* source, size_t len) {
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
void KarplusStrong::_fillDelayFiltered(float* source, size_t len) {
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

float KarplusStrong::_impulseFilters(float input) {
    float out = input;
    if(_impulsePickPosOn) {
        out -= _impulseDelay.read();
        _impulseDelay.push(clamp(out, -1.f, 1.f));
    }

    if(_impulseDynamicsOn) {
        // this doesn't simulate proper dynamics w R, but is interesting
        out = _impulseLPF.process(out);
    }
    return out;
}

// --------------------- Thread Related ---------------------------------------
// ----------------------------------------------------------------------------

void KarplusStrong::_startImpulseThread() {
    _impulseThread  = std::thread(&KarplusStrong::_impulseWorker, this);
}


void KarplusStrong::_killImpulseThread() {
    _keepWorking = false;
}



void KarplusStrong::_startImpulseJob(int type) {
    _impulseType = type;
}


void KarplusStrong::_impulseWorker() {
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

// STATIC method called by the worker thread to load a wavefile
WavFilePtr KarplusStrong::__loadImpulseFile(int fileNum) {
    WavFilePtr wf = __wavefiles.getWavFile(fileNum);
    wf->load();
    return wf;
}

// --------------------- Dust bin below ---------------------------------------
// ----------------------------------------------------------------------------


/*
    * This is for the "dynamics" section of the Jaffe-Smith extensions
    * to the Karplus-Strong string model.
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