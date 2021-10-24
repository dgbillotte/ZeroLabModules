#include "../plugin.hpp"
#include "../../dep/dr_wav.h"
#include "KarplusStrong.hpp"

WaveFile wavefiles[10] = {
    WaveFile("res/white-noise-1000-samples.wav"),
    WaveFile("res/pink-noise-1000-samples.wav"),
    WaveFile("res/brownian-noise-1000-samples.wav"),
    WaveFile("res/sine40.wav"),
    WaveFile("res/sine100.wav"),
    WaveFile("res/sine500.wav"),
    WaveFile("res/sine1000.wav"),
    WaveFile("res/sine_256.wav"),
    WaveFile("res/sine-chirp-10k-samples.wav"),
    WaveFile("res/sqr-chirp-5k-samples.wav")
};

int KarplusStrong::__numInstances = 0;

WaveFile KarplusStrong::__wavefiles[5] = {
    WaveFile("res/white-noise-1000-samples.wav"),
    WaveFile("res/sine100.wav"),
    WaveFile("res/sine500.wav"),
    WaveFile("res/sine1000.wav"),
    WaveFile("res/sine_256.wav"),
    // WaveFile("res/pink-noise-1000-samples.wav"),
    // WaveFile("res/brownian-noise-1000-samples.wav"),
    // WaveFile("res/sine40.wav"),
    // WaveFile("res/sine-chirp-10k-samples.wav"),
    // WaveFile("res/sqr-chirp-5k-samples.wav")
};

KarplusStrong::~KarplusStrong() {
    _killImpulseThread();
    while(! _impulseThread.joinable()) {
        std::this_thread::yield();
    }
    _impulseThread.join();
}
// KarplusStrong::~KarplusStrong() {
//     // last one out cleans up the shared resources
//     if(--__numInstances == 0)
//         for(size_t i=0; i < sizeof(wavefiles); i++)
//             if(wavefiles[i].wavetable != NULL)
//                 drwav_free(wavefiles[i].wavetable, NULL);
// }



WaveFile& KarplusStrong::_loadImpulseFile(int fileNum) {
    // load an impulse file
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 numSamples;

    WaveFile& wf = __wavefiles[fileNum];
    if(wf.wavetable == NULL) {
        auto fullpath = asset::plugin(pluginInstance, wf.filename);
        wf.wavetable = drwav_open_file_and_read_pcm_frames_f32(fullpath.c_str(), &channels, &sampleRate, &numSamples, NULL);
        if (wf.wavetable == NULL) {
            std::cerr << "Unable to open file: " << fullpath << std::endl;
            wf.numSamples = 0;
        } else {
            wf.numSamples = numSamples;
        }
    }

    return wf;
}