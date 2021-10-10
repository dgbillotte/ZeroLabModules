#include <iostream>
#define DR_WAV_IMPLEMENTATION
#include "../../dep/dr_wav.h"

int main() {

    const char* wavefile = "/Users/daniel/projects/waveforms/sine_256.wav";

    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalPCMFrameCount;
    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(wavefile, &channels, &sampleRate, &totalPCMFrameCount, NULL);
    if (pSampleData == NULL) {
        std::cerr << "Unable to open file: " << wavefile << std::endl;
    }

    std::cout << "File opened. Details:" << std::endl;
    std::cout << "\tchannels: " << channels << std::endl;
    std::cout << "\tsample rate: " << sampleRate << std::endl;
    std::cout << "\tframe count: " << totalPCMFrameCount << std::endl;
    
    for(unsigned int i=0; i < totalPCMFrameCount; i++) {
        std::cout << pSampleData[i] << ", ";
        if(i % 5 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;

    drwav_free(pSampleData, NULL);


}