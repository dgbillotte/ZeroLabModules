#ifndef LUT_HPP
#define LUT_HPP

#include <mutex>
#include <vector>

class LUT;
typedef std::shared_ptr<LUT> LUTPtr;

class LUT {
    const int _numEntries;
    const float _firstX;
    float _inc;
    bool _loaded = false;
    std::vector<float> _table;
    std::mutex _loadingMutex;    
    std::function<float(float)> _f;
    
public:
    LUT(float x0, float xN, int numEntries, float (*f)(float), bool loadNow=true) :
        _numEntries(numEntries),
        _firstX(x0),
        _inc((xN - x0) / (numEntries-1)),
        _f(f)
    {
        if(loadNow) {
            std::cout << "ctor loading the LUT" << std::endl;
            load();
        }
    }

    void load() {
        if(_loaded)
            return;
        std::unique_lock<std::mutex> lock(_loadingMutex);
        if(! _loaded) {
            float x = _firstX;
            for(int i=0; i < _numEntries; i++) {
                _table.push_back(_f(x));
                x += _inc;
            }
            _loaded = true;
        }
    }

    float at(float x) {
        load();
        float fIdx = (x - _firstX) / _inc;
        int x0 =(int)fIdx;
        int x1 = (x0 + 1 < _numEntries) ? x0 + 1 : 0;
        float y0 = _table.at(x0);
        float y1 = _table.at(x1);
        return y0 + ((y1 - y0) * (fIdx - x0));        
    }

    float atIdx(int idx) {
        load();
        return _table[idx];
    }

    size_t size() { return _numEntries; }
    float firstX() { return _firstX; };

protected:
    void _pushEntry(float value);
};

#endif