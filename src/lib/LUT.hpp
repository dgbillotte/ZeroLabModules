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
    std::vector<float> _table;

public:
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

    inline float at(float x) {
        float fIdx = (x - _firstX) / _inc;
        int x0 =(int)fIdx;
        int x1 = (x0 + 1 < _numEntries) ? x0 + 1 : 0;
        float y0 = _table.at(x0);
        float y1 = _table.at(x1);
        return y0 + ((y1 - y0) * (fIdx - x0));        
    }

    inline float atIdx(int idx) {
        return _table[idx];
    }

    inline size_t size() { return _numEntries; }
    inline float firstX() { return _firstX; };
    inline float lastX() { return _lastX; }


};

#endif