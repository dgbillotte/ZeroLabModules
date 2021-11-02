#include "WavFileStore.hpp"


void WavFileStore::_addFile(const char* filename) {
    const auto wf = WavFilePtr(new WavFile(filename));
    _waveFiles.push_back(wf);
}

WavFileStore::WavFileStore(const char** files, size_t numFiles) {
    for(size_t i=0; i < numFiles; i++) {
        _addFile(files[i]);
    }
}

WavFilePtr WavFileStore::getWavFile(int index) {
    auto wf = _waveFiles.at(index);
    wf->load();
    return wf;
}

size_t WavFileStore::size() {
    return _waveFiles.size();
}








