#ifndef WAV_FILE_STORE_HPP
#define WAV_FILE_STORE_HPP
#include <mutex>
#include <vector>
#include "WavFile.hpp"

class WavFileStore {
    // static WavFilePtr __wavefiles[];
    static std::vector<WavFilePtr> __waveFiles;
public:
    WavFileStore(const char** files, size_t numFiles) {
        for(size_t i=0; i < numFiles; i++) {
            std::cout << "adding file to store: " << files[i] << std::endl;
            addFile(files[i]);
        }
    }

    static WavFilePtr& getWavFile(int index);
    static void addFile(const char* filename) {
        // const auto wf = std::shared_ptr<WavFile>(new WavFile(filename));
        const auto wf = WavFilePtr(new WavFile(filename));
        __waveFiles.push_back(wf);
    }

    static size_t size() {
        return __waveFiles.size();
    }
};

#endif
