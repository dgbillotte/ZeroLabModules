#include "../plugin.hpp"
// #include "../../dep/dr_wav.h"
#include "KarplusStrong.hpp"

int KarplusStrong::__numInstances = 0;

std::unique_ptr<WavFile> KarplusStrong::__wavefiles[5] = {
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



KarplusStrong::~KarplusStrong() {
    _killImpulseThread();
    while(! _impulseThread.joinable()) {
        std::this_thread::yield();
    }
    _impulseThread.join();


    // last one out cleans up the shared resources
    //
    // not sure why this is crashing. Getting following error:
    // Rack(25515,0x112e79e00) malloc: *** error for object 0x10cea008: pointer being freed was not allocated
    // Rack(25515,0x112e79e00) malloc: *** set a breakpoint in malloc_error_break to debug
    // if(--__numInstances == 0)
    //     for(size_t i=0; i < sizeof(__wavefiles); i++)
    //         __wavefiles[i]->unload();

}


WavFilePtr& KarplusStrong::_loadImpulseFile(int fileNum) {
    WavFilePtr& wf = __wavefiles[fileNum];
    wf->load();
    return wf;
}

