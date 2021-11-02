#include "WavFileStore.hpp"
#include "../plugin.hpp"
#include "WavFile.hpp"

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