#include "../plugin.hpp"

#include "KarplusStrong.hpp"
#include "WavFileStore.hpp"

// static members
int KarplusStrong::__numInstances = 0;


const char* files[5] = {
    "res/white-noise-1000-samples.wav",
    "res/sine100.wav",
    "res/sine500.wav",
    "res/sine1000.wav",
    "res/sine_256.wav"
};

WavFileStore KarplusStrong::__wavefiles(files, 5);


KarplusStrong::~KarplusStrong() {
    _killImpulseThread();
    while(! _impulseThread.joinable()) {
        std::this_thread::yield();
    }
    _impulseThread.join();

    if(--__numInstances == 0) {
        __wavefiles.clearCache();
    }
}


WavFilePtr KarplusStrong::__loadImpulseFile(int fileNum) {
    WavFilePtr wf = __wavefiles.getWavFile(fileNum);
    wf->load();
    return wf;
}
