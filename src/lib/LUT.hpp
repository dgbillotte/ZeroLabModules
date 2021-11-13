#ifndef LUT_HPP
#define LUT_HPP

#include <vector>

class LUT;
typedef std::shared_ptr<LUT> LUTPtr;

class LUT {
    const int _numEntries;
    const float _firstX;
    // float _lastX; // TODO: I think we can get rid of this
    float _inc;
    std::vector<float> _table;

public:
    LUT(float x0, float xN, int numEntries, float (*f)(float)) :
        _numEntries(numEntries),
        _firstX(x0),
        _inc((xN - x0) / (numEntries-1))
    {
        float x = x0;
        for(int i=0; i < numEntries; i++) {
            _table.push_back(f(x));
            x += _inc;
        }
        // _lastX = x;
    }

    float at(float x) {
        float fIdx = (x - _firstX) / _inc;
        int x0 =(int)fIdx;
        int x1 = (x0 + 1 < _numEntries) ? x0 + 1 : 0;
        float y0 = _table.at(x0);
        float y1 = _table.at(x1);
        return y0 + ((y1 - y0) * (fIdx - x0));        
    }

    float atIdx(int idx) {
        return _table[idx];
    }

    size_t size() { return _numEntries; }
    float firstX() { return _firstX; };
    // float lastX() { return _lastX; };

protected:
    void _pushEntry(float value);
};

#endif