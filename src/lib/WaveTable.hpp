#ifndef WAVE_TABLE_HPP
#define WAVE_TABLE_HPP

#include <mutex>
#include <vector>

class WaveTable;
typedef std::shared_ptr<WaveTable> WaveTablePtr;
typedef std::function<void()> WaveTableLoader;

struct WaveSpecLength {
    std::string name;
    size_t numSamples;
    std::function<float(float)> f;
    float x0 = 0.f;
    float xN = 1.f;

    WaveSpecLength(std::string name, size_t numSamples, std::function<float(float)> f, float x0=0.f, float xN=1.f) :
        name(name),
        numSamples(numSamples),
        f(f),
        x0(x0),
        xN(xN) {}
};

struct WaveSpecFreq {
    std::string name;
    float freq;
    size_t sampleRate;
    std::function<float(float)> f;
    float x0 = 0.f;
    float xN = 1.f;

    WaveSpecFreq(std::string name, float freq, size_t sampleRate, std::function<float(float)> f, float x0=0.f, float xN=1.f) :
        name(name),
        freq(freq),
        sampleRate(sampleRate),
        f(f),
        x0(x0),
        xN(xN) {}
};

//-----------------------------------------------------------------------------
class WaveTable {
    std::vector<float> _wavetable;
    size_t _numSamples = 0;

public:
    /*
     * Two Constructors:
     * - both take x0, xN, & f.
     * - the first takes numSamples and produces a wavetable of length
     *   numSamples for the given function over [x0..xN) 
     * - the second takes freq and sampleRate to produce a wavetable that
     *   is (sampleRate / (xN - x0) * freq) long that can be walked in
     *   integer steps at the given sampleRate and frequency
     */
    WaveTable(WaveSpecLength& spec) :
        _numSamples(spec.numSamples)
    {
        float inc = (spec.xN - spec.x0) / _numSamples;
        for(float x = spec.x0; x < spec.xN;x += inc) {
            _wavetable.push_back(spec.f(x));
        }   
    }

    WaveTable(WaveSpecFreq& spec) :
        _numSamples(spec.sampleRate/spec.freq)
    {
        float inc = (spec.xN - spec.x0) * spec.freq / spec.sampleRate;
        for(float x = spec.x0; x < spec.xN;x += inc) {
            _wavetable.push_back(spec.f(x));
        }
    }
    

    inline size_t size() { return _numSamples; }

    inline float at(int idx) { return _wavetable.at(idx); }

    inline float atF(float idxF){
        size_t x0 = (int)idxF;
        size_t x1 = (x0 + 1 < _numSamples) ? x0 + 1 : 0;
        float y0 = _wavetable.at(x0);
        float y1 = _wavetable.at(x1);

        return y0 + ((y1 - y0) * (idxF - x0));
    }

    void dump(int cols=5) {
        std::cout << "--- begin waveform" << std::endl;
        for(auto it=_wavetable.begin(); it < _wavetable.end(); ++it) {
            for(int i=0; i < cols && it < _wavetable.end(); i++, ++it) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "--- end waveform" << std::endl;
    }

};


#endif
