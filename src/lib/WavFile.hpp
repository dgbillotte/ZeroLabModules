#ifndef WAVE_FILE_HPP
#define WAVE_FILE_HPP
#include "../../dep/dr_wav.h"
#include "../plugin.hpp"

/*
 * class: WavFile
 * what: simple "thread-safe" container for a filesystem-based wav-file.
 * holds: fullpath, the samples from the file, and the number of samples.
 * If used read-only, can be safely loaded/unloaded my multiple threads


 * To do:
 * - change to a class
 * - make read only
 * - add gain member to accommodate low/high gain files
 */


struct WavFile {
    const char* filename;
    float* _wavetable = nullptr;
    int _numSamples;
    std::mutex _loadingMutex;
    // float gainTo1 = 1.f;

    WavFile(const char* filename) {
        this->filename = filename;
    }

    ~WavFile() {
        unload();
    }
    
    float* waveTable() {
        return _wavetable;
    }

    int numSamples() {
        return _numSamples;
    }

    void load() {
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 numSamples;
std::cout << "inside load" << std::endl;
        std::unique_lock<std::mutex> lock(_loadingMutex);

        if(_wavetable == nullptr) {
            auto fullpath = asset::plugin(pluginInstance, filename);
            _wavetable = drwav_open_file_and_read_pcm_frames_f32(fullpath.c_str(), &channels, &sampleRate, &numSamples, nullptr);
            if (_wavetable == nullptr) {
                std::cerr << "Unable to open file: " << fullpath << std::endl;
                _numSamples = 0;
            } else {
                _numSamples = numSamples;
            }
        }
    }

    void unload() {
        std::unique_lock<std::mutex> lock(_loadingMutex);

        if(_wavetable != nullptr) {
            drwav_free(_wavetable, nullptr);
        }
    }
};

typedef std::shared_ptr<WavFile> WavFilePtr;
#endif