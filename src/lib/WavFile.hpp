#include "../../dep/dr_wav.h"

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
    float* wavetable = nullptr;
    int numSamples;
    std::mutex loadingMutex;
    // float gainTo1 = 1.f;

    WavFile(const char* filename) {
        this->filename = filename;
    }

    ~WavFile() {
        unload();
    }

    void load() {
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 numSamples;

        std::unique_lock<std::mutex> lock(loadingMutex);

        if(wavetable == nullptr) {
            auto fullpath = asset::plugin(pluginInstance, filename);
            wavetable = drwav_open_file_and_read_pcm_frames_f32(fullpath.c_str(), &channels, &sampleRate, &numSamples, nullptr);
            if (wavetable == nullptr) {
                std::cerr << "Unable to open file: " << fullpath << std::endl;
                this->numSamples = 0;
            } else {
                this->numSamples = numSamples;
            }
        }
    }

    void unload() {
        std::unique_lock<std::mutex> lock(loadingMutex);

        if(wavetable != nullptr) {
            drwav_free(wavetable, nullptr);
        }
    }
};