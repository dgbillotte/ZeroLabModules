#ifndef OBJECT_STORE_HPP
#define OBJECT_STORE_HPP

#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// #include "WavFile.hpp"

using namespace std;

//-----------------------------------------------------------------------------
// struct WaveTable {
//     float* _wavetable = nullptr;
//     int _numSamples = 0;
// };

// typedef std::shared_ptr<WaveTable> WaveTablePtr;

//-----------------------------------------------------------------------------
class LUT {
    const int _numEntries;
    const float _firstX;
    float _lastX;
    float _inc;
    std::vector<float> _table;

public:
    LUT(float x0, float xN, int numEntries, float (*f)(float)) :
        _numEntries(numEntries),
        _firstX(x0),
        _inc((xN - x0) / (numEntries-1)) {
        float x = x0;
        for(int i=0; i < numEntries; i++) {
            _table.push_back(f(x));
            x += _inc;
        }
        _lastX = x;
    }

    float at(float x) {
        float fIdx = (x - _firstX) / _inc;
        int x0 =(int)fIdx;
        int x1 = (x0 + 1 < _numEntries) ? x0 + 1 : 0;
        float y0 = _table[x0];
        float y1 = _table[x1];
        return y0 + ((y1 - y0) * (fIdx - x0));        
    }

    float atIdx(int idx) {
        return _table[idx];
    }

    size_t size() { return _numEntries; }
    float firstX() { return _firstX; };
    float lastX() { return _lastX; };

protected:
    void _pushEntry(float value);
};
typedef std::shared_ptr<LUT> LUTPtr;


 

//-----------------------------------------------------------------------------

// typedef std::shared_ptr<WavFile> WaveTablePtr;
// typedef std::map<std::string, WaveTablePtr> WaveTableStore;
// typedef WaveTableStore::iterator wavetable_iterator;
// typedef std::pair<std::string, WaveTablePtr> wavetable_pair;

typedef std::map<std::string, LUTPtr> LUTStore;
typedef LUTStore::iterator lut_iterator;
typedef std::pair<std::string, LUTPtr> lut_pair;


class ObjectStore;
typedef shared_ptr<ObjectStore> ObjectStorePtr;

/*
 * Standard Wavefiles
 * SIN_1024
 * TRI_1024
 * SAW_1024
 * SQUARE_1024
 * WHITE_NOISE_1024
 * RANDOM_SQUARE_1024
 * SIN_2ND
 * SIN_3RD
 * SIN_4TH
 * SIN_5TH
 * SIN_6TH
 * SIN_7TH
 * 
 * Standard LUTs
 * SIN_0_2PI_1024
 * COS_0_2PI_1024
 * TAN
 * ATAN
 * EXPD
 * 
 */


class ObjectStore {
    // WaveTableStore _wavetables;
    LUTStore _luts;

    static ObjectStorePtr __theStore;
    static std::mutex __loadingMutex;
    ObjectStore() {}

public:
    static ObjectStorePtr getStore() {
        std::unique_lock<std::mutex> lock(__loadingMutex);
        if(__theStore == nullptr) {
            __theStore = ObjectStorePtr(new ObjectStore());
        }
        return __theStore;
    }

    // WaveTablePtr loadWavetable(std::string key, std::string filename) {
    //     wavetable_iterator it = _wavetables.find(key);
    //     if(it != _wavetables.end()) {
    //         return it->second;
    //     }

    //     WaveTablePtr wavetable = WaveTablePtr(new WavFile(filename.c_str()));
    //     _wavetables.insert(wavetable_pair(key, wavetable));
    //     return wavetable;
    // }

    // WaveTablePtr getWavetable(std::string key) {
    //     wavetable_iterator it = _wavetables.find(key);
    //     if(it != _wavetables.end()) {
    //         return it->second;
    //     }
    //     return nullptr;
    // }


    LUTPtr loadLUT(std::string key, float x0, float xN, int numEntries, float (*f)(float)) {
        lut_iterator it = _luts.find(key);
        if(it != _luts.end()) {
            // std::cout << "using cached LUT" << std::endl;
            return it->second;
        }

        // std::cout << "!!! --- creating new LUT" << std::endl;
        LUTPtr lut = LUTPtr(new LUT(x0, xN, numEntries, f));
        _luts.insert(lut_pair(key, lut));
        return lut;
    }

    LUTPtr getLUT(std::string key) {
        lut_iterator it = _luts.find(key);
        if(it != _luts.end()) {
            return it->second;
        }

        return nullptr;
    }


};

#endif