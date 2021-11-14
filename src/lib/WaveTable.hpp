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
    float _inc;
    std::mutex _loadingMutex;
    bool _loaded = false;
    std::function<float(float)> _f;
    float _x0;
    float _xN;

public:
    WaveTable(float x0, float xN, size_t numSamples, float (*f)(float), bool loadNow=true) :
        _numSamples(numSamples),
        _inc((xN - x0) / numSamples),
        _f(f),
        _x0(x0),
        _xN(xN)
    {
        if(loadNow) {
            load();
        }
    }

    void load() {
        if(_loaded)
            return;
        std::unique_lock<std::mutex> lock(_loadingMutex);
        if(! _loaded) {
            for(float x = _x0; x < _xN; x += _inc) {
                _wavetable.push_back(_f(x));
            }    
        }
        _loaded = true;
    }

    float at(float x) {
        load();
        float fIdx = x / _inc;
        size_t x0 = (size_t)fIdx;
        size_t x1 = (x0 + 1 < _numSamples) ? x0 + 1 : 0;
        float y0 = _wavetable.at(x0);
        float y1 = _wavetable.at(x1);

        return y0 + ((y1 - y0) * (fIdx - x0));
    }

    float atIdx(size_t idx) {
        load();
        return _wavetable.at(idx);
    }

    size_t size() { return _numSamples; }
};


#endif
