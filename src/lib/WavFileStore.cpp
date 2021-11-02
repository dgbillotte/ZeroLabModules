#include "WavFileStore.hpp"
#include "../plugin.hpp"
#include "WavFile.hpp"

// WavFilePtr WavFileStore::__wavefiles[5] = {
//     std::unique_ptr<WavFile>(new WavFile("res/white-noise-1000-samples.wav")),
//     std::unique_ptr<WavFile>(new WavFile("res/sine100.wav")),
//     std::unique_ptr<WavFile>(new WavFile("res/sine500.wav")),
//     std::unique_ptr<WavFile>(new WavFile("res/sine1000.wav")),
//     std::unique_ptr<WavFile>(new WavFile("res/sine_256.wav")),
//     // WavFile("res/pink-noise-1000-samples.wav"),
//     // WavFile("res/brownian-noise-1000-samples.wav"),
//     // WavFile("res/sine40.wav"),
//     // WavFile("res/sine-chirp-10k-samples.wav"),
//     // WavFile("res/sqr-chirp-5k-samples.wav")
// };


// const char* files[5] = {
//     "res/white-noise-1000-samples.wav",
//     "res/sine100.wav",
//     "res/sine500.wav",
//     "res/sine1000.wav",
//     "res/sine_256.wav"
// };
// std::vector<WavFilePtr> WavFileStore::__waveFiles(&files, 5);
std::vector<WavFilePtr> WavFileStore::__waveFiles;

// WavFileStore::addFile("res/white-noise-1000-samples.wav");
// WavFileStore::addFile("res/sine100.wav");
// WavFileStore::addFile("res/sine500.wav");
// WavFileStore::addFile("res/sine1000.wav");
// WavFileStore::addFile("res/sine_256.wav");

WavFilePtr& WavFileStore::getWavFile(int index) {
    auto wf = __waveFiles[index];
    std::cout << "got here, index: " << index << std::endl;
    wf->load();
    return wf;
}

