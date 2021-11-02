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

    __freeWavFiles();
}


WavFilePtr KarplusStrong::__loadImpulseFile(int fileNum) {
    WavFilePtr wf = __wavefiles.getWavFile(fileNum);
    wf->load();
    return wf;
}


void KarplusStrong::__freeWavFiles() {
    __numInstances--;

    // last one out cleans up the shared resources
    //
    // not sure why this is crashing. Getting following error:
    // Rack(25515,0x112e79e00) malloc: *** error for object 0x10cea008: pointer being freed was not allocated
    // Rack(25515,0x112e79e00) malloc: *** set a breakpoint in malloc_error_break to debug
    // if(__numInstances <= 0) {
    //     std::cout << "last one out, I got clean up" << std::endl;
    //     for(size_t i=0; i < (sizeof(__wavefiles)/sizeof(WavFilePtr)); i++) {
    //         std::cout << "value of pointer" << __wavefiles[i] << std::endl;
    //         __wavefiles[i]->unload();
    //     }
    // } else {
    //     std::cout << "not it!" << std::endl;
    // }
    
}