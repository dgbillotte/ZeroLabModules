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

void KarplusStrong::setOTFSample(float sample) {
    _externalOTFSample = sample;
    _haveExternalOTFSample = true;
}

void KarplusStrong::setImpulseWavetable(float* wt, size_t numSamples) {
    _externalImpulseWavetable = wt;
    _externalImpulseNumSamples = numSamples;
}

float KarplusStrong::nextValue() {
    // float x0 = _delayLine.read();

    // _excite();


    float x0 = _excite();
    if(_attack_on > 0) {
        return x0;
    }

    // if attack is on, run the impulse filters and store the value back into the delay
    // if(_attack_on > 0) {
    //     float y0 = x0; //_impulseFilters(x0);
    //     _delayLine.write(-1, y0);

    //     // if on-the-fly is not running
    //     if(_write_i == _delayLength) {
    //         _delayLine.push(y0);
    //     }
    //     return y0;
    // }

    // read last two items from the delay
    float y0 = _delayLine.read(-1);
    float y1 = _delayLine.read(-2);

    // KP with a 2-point weighted average
    float out = (x0 + _p*((1-_S)*y0 + _S*y1))/2; // the 2nd /2 isn't mentioned, but it blows up without it....
    _delayLine.push(out);

    // all-pass filter to correct tuning
    float Pc = 0.5f;
    float C = (1 - Pc) / (1 + Pc);
    out = C * out + x0 - C * y0;

    return out;
}


// --------------------- Private Methods --------------------------------------
// ----------------------------------------------------------------------------

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