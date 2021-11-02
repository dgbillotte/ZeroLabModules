#include "../plugin.hpp"
#include "WavFile.hpp"

// ----------- WavFile Implementation -----------------------------------------

WavFile::WavFile(const char* filename) {
    _filename = filename;
}

WavFile::~WavFile() {
    unload();
}

inline const char* WavFile::filename() { return _filename; }
inline float* WavFile::waveTable() { return _wavetable; }
inline int WavFile::numSamples() { return _numSamples; }

void WavFile::load() {
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 numSamples;

    std::unique_lock<std::mutex> lock(_loadingMutex);

    if(_wavetable == nullptr) {
        auto fullpath = asset::plugin(pluginInstance, _filename);
        _wavetable = drwav_open_file_and_read_pcm_frames_f32(fullpath.c_str(), &channels, &sampleRate, &numSamples, nullptr);
        if (_wavetable == nullptr) {
            std::cerr << "Unable to open file: " << fullpath << std::endl;
            _numSamples = 0;
        } else {
            _numSamples = numSamples;
        }
    }
}

void WavFile::unload() {
    std::unique_lock<std::mutex> lock(_loadingMutex);

    if(_wavetable != nullptr) {
        drwav_free(_wavetable, nullptr);
    }
}


// ----------- WavFileStore Implementation ------------------------------------
WavFilePtr WavFileStore::__wavefiles[5] = {
    std::unique_ptr<WavFile>(new WavFile("res/white-noise-1000-samples.wav")),
    std::unique_ptr<WavFile>(new WavFile("res/sine100.wav")),
    std::unique_ptr<WavFile>(new WavFile("res/sine500.wav")),
    std::unique_ptr<WavFile>(new WavFile("res/sine1000.wav")),
    std::unique_ptr<WavFile>(new WavFile("res/sine_256.wav")),
    // WavFile("res/pink-noise-1000-samples.wav"),
    // WavFile("res/brownian-noise-1000-samples.wav"),
    // WavFile("res/sine40.wav"),
    // WavFile("res/sine-chirp-10k-samples.wav"),
    // WavFile("res/sqr-chirp-5k-samples.wav")
};

WavFilePtr& WavFileStore::getWavFile(int index) {
    WavFilePtr& wf = __wavefiles[index];
    wf->load();
    return wf;
}