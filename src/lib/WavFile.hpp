#ifndef WAVE_FILE_HPP
#define WAVE_FILE_HPP
#include <mutex>
#include "../../dep/dr_wav.h"
#include "../plugin.hpp"

/*
 * class: WavFile
 * what: simple "thread-safe" container for a filesystem-based wav-file.
 * holds: fullpath, the samples from the file, and the number of samples.
 * If used read-only, can be safely loaded/unloaded my multiple threads


 * To do:
 * - make read only
 * - add gain member to accommodate low/high gain files
 */

class WavFile;
typedef std::unique_ptr<WavFile> WavFilePtr;

class WavFile {
    const char* _filename;
    float* _wavetable = nullptr;
    int _numSamples;
    std::mutex _loadingMutex;
    // float gainTo1 = 1.f;

public:
    WavFile(const char* filename);
    ~WavFile();

    const char* filename();
    float* waveTable();
    int numSamples();

    void load();
    void unload();
};

class WavFileStore {
    static WavFilePtr __wavefiles[];
public:
    static WavFilePtr& getWavFile(int index);
};

#endif