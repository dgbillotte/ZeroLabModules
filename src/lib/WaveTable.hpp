#ifndef WAVE_TABLE_HPP
#define WAVE_TABLE_HPP

#include <vector>

// using namespace std;

class WaveTable;
typedef std::shared_ptr<WaveTable> WaveTablePtr;

//-----------------------------------------------------------------------------
class WaveTable {
    std::vector<float> _wavetable;
    size_t _numSamples = 0;
    float _inc;
    // void (*_loader)() = NULL;
    // function<void()>* _loader = NULL;
    // mutex _loadingMutex;

public:
    WaveTable(float x0, float xN, size_t numSamples, float (*f)(float) /*, bool loadNow=true*/) :
        _numSamples(numSamples),
        _inc((xN - x0) / numSamples)
    {
        bool loadNow = true;
        if(loadNow) {
            int i = 0;
            for(float x = x0; x < xN; x += _inc) {
                float y = f(x);
                _wavetable.push_back(y);
                i++;
            }    
        } else {
            // _loader = [this, x0, xN, f]() {
            //     for(float x = x0; x < xN; x += _inc) {
            //         _wavetable.push_back((f(x)));
            //         x += _inc;
            //     }
            //     _loader = NULL;
            // }
        }
    }

    // void load(){
    //     if(_loader == NULL)
    //         return;
    //     unique_lock<mutex> lock(_loadingMutex);
    //     if(_loader != NULL)
    //         _loader();
    // }

    float at(float x) {
        // load();
        float fIdx = x / _inc;
        size_t x0 = (size_t)fIdx;
        size_t x1 = (x0 + 1 < _numSamples) ? x0 + 1 : 0;
        float y0 = _wavetable.at(x0);
        float y1 = _wavetable.at(x1);

        return y0 + ((y1 - y0) * (fIdx - x0));
    }

    float atIdx(size_t idx) {
        // load();
        return _wavetable.at(idx);
    }

    size_t size() { return _numSamples; }
};


#endif
