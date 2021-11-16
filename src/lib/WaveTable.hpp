#ifndef WAVE_TABLE_HPP
#define WAVE_TABLE_HPP

#include <mutex>
#include <vector>

class WaveTable;
typedef std::shared_ptr<WaveTable> WaveTablePtr;
typedef std::function<void()> WaveTableLoader;

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
    WaveTable(size_t numSamples, float (*f)(float), float x0=0.f, float xN=1.f) :
        _numSamples(numSamples)
    {
        float inc = (xN - x0) / numSamples;
        for(float x = x0; x < xN; x += inc) {
            _wavetable.push_back(f(x));
        }
    }

    WaveTable(float freq, float sampleRate, float (*f)(float), float x0=0.f, float xN=1.f) {
        float inc = (xN - x0) * freq / sampleRate;
        for(float x = x0; x < xN; x += inc) {
            _wavetable.push_back(f(x));
        }
        _numSamples = _wavetable.size();
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
