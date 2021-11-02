#ifndef WAV_FILE_STORE_HPP
#define WAV_FILE_STORE_HPP
#include <vector>
#include "WavFile.hpp"

class WavFileStore {
    std::vector<WavFilePtr> _waveFiles;

public:
    WavFileStore(const char** files, size_t numFiles);
    WavFilePtr getWavFile(int index);
    size_t size();
    
protected:
    void _addFile(const char* filename);
};

#endif
