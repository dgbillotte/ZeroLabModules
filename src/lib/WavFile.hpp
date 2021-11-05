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
    const char* _filename;
    float* _wavetable = NULL;
    int _numSamples;
    std::mutex _loadingMutex;
    // float gainTo1 = 1.f;

    WavFile(const char* filename) :
        _filename(filename) {}

    WavFile(float* wavetable, size_t numSamples) :
        _filename("external"),
        _wavetable(wavetable),
        _numSamples(numSamples) {}

    ~WavFile() {
        unload();
    }
    
    float* waveTable() {
        return _wavetable;
    }

    int numSamples() {
        return _numSamples;
    }

    const char* filename() {
        return _filename;
    }

    void load() {
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 numSamples;

        std::unique_lock<std::mutex> lock(_loadingMutex);
        if(_wavetable == NULL) {
            auto fullpath = asset::plugin(pluginInstance, _filename);
            _wavetable = drwav_open_file_and_read_pcm_frames_f32(fullpath.c_str(), &channels, &sampleRate, &numSamples, NULL);
            if (_wavetable == NULL) {
                std::cerr << "Unable to open file: " << fullpath << std::endl;
                _numSamples = 0;
            } else {
                _numSamples = numSamples;
            }
        }
    }

    void unload() {
        std::unique_lock<std::mutex> lock(_loadingMutex);

        if(_wavetable != NULL) {
            drwav_free(_wavetable, NULL);
            _wavetable = NULL;
        }
    }
};

typedef std::shared_ptr<WavFile> WavFilePtr;
#endif