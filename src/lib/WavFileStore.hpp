#ifndef WAV_FILE_STORE_HPP
#define WAV_FILE_STORE_HPP
#include <mutex>
#include "WavFile.hpp"

class WavFileStore {
    static WavFilePtr __wavefiles[];
public:
    static WavFilePtr& getWavFile(int index);
};

#endif