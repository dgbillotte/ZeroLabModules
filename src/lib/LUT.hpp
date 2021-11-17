#ifndef LUT_HPP
#define LUT_HPP

#include <mutex>
#include <vector>

class LUT;
typedef std::shared_ptr<LUT> LUTPtr;

struct LUTSpec {
    std::string name;
    size_t length;
    std::function<float(float)> f;
    float x0 = 0.f;
    float xN = 1.f;

    LUTSpec(std::string name, size_t length, std::function<float(float)> f, float x0=0.f, float xN=1.f) :
        name(name),
        length(length),
        f(f),
        x0(x0),
        xN(xN) {}
};

class LUT {
    const int _numEntries;
    const float _firstX;
    const float _lastX;
    float _inc;
    // bool _loaded = false;
    std::vector<float> _table;
    // std::mutex _loadingMutex;    
    // std::function<float(float)> _f;

public:
    LUT(float x0, float xN, int numEntries, float (*f)(float), bool loadNow=true) :
        _numEntries(numEntries),
        _firstX(x0),
        _lastX(xN),
        _inc((xN - x0) / (numEntries-1))
        // _f(f)
    {
        float x = _firstX;
        for(int i=0; i < _numEntries; i++) {
            _table.push_back(f(x));
            x += _inc;
        }

        // if(loadNow) {
        //     std::cout << "ctor loading the LUT" << std::endl;
        //     load();
        // }
    }

    LUT(LUTSpec& spec) :
        _numEntries(spec.length),
        _firstX(spec.x0),
        _lastX(spec.xN),
        _inc((spec.xN - spec.x0) / (spec.length - 1))    
    {
        float x = spec.x0;
        for(int i=0; i < _numEntries; i++) {
            _table.push_back(spec.f(x));
            x += _inc;
        }

    }

    // void load() {
    //     if(_loaded)
    //         return;
    //     std::unique_lock<std::mutex> lock(_loadingMutex);
    //     if(! _loaded) {
    //     }
    // }

    float at(float x) {
        // load();
        float fIdx = (x - _firstX) / _inc;
        int x0 =(int)fIdx;
        int x1 = (x0 + 1 < _numEntries) ? x0 + 1 : 0;
        float y0 = _table.at(x0);
        float y1 = _table.at(x1);
        return y0 + ((y1 - y0) * (fIdx - x0));        
    }

    float atIdx(int idx) {
        // load();
        return _table[idx];
    }

    size_t size() { return _numEntries; }
    float firstX() { return _firstX; };
    float lastX() { return _lastX; }

protected:
    void _pushEntry(float value);
};

#endif