#ifndef OBJECT_STORE_HPP
#define OBJECT_STORE_HPP

#include <iostream>
#include <map>
#include <mutex>
#include <string>


#include "WaveTable.hpp"
#include "LUT.hpp"

using namespace std;


 

//-----------------------------------------------------------------------------

typedef std::map<std::string, WaveTablePtr> WaveTableStore;
typedef WaveTableStore::iterator wavetable_iterator;
typedef std::pair<std::string, WaveTablePtr> wavetable_pair;

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
    WaveTableStore _wavetables;
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

    WaveTablePtr loadWavetable(WaveSpecLength& spec) {
        wavetable_iterator it = _wavetables.find(spec.name);
        if(it != _wavetables.end()) {
            return it->second;
        }
        
        WaveTablePtr wavetable = WaveTablePtr(new WaveTable(spec));
        _wavetables.insert(wavetable_pair(spec.name, wavetable));
        return wavetable;
    }

    WaveTablePtr getWavetable(std::string key) {
        wavetable_iterator it = _wavetables.find(key);
        if(it != _wavetables.end()) {
            return it->second;
        }
        return nullptr;
    }

    LUTPtr loadLUT(LUTSpec& spec) {
        lut_iterator it = _luts.find(spec.name);
        if(it != _luts.end()) {
            return it->second;
        }

        LUTPtr lut = LUTPtr(new LUT(spec));
        _luts.insert(lut_pair(spec.name, lut));
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