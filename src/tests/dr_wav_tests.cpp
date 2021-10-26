#include <iostream>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


#include "WavFile.hpp"

// int DUMP = true;
// #include "dump.hpp"


int main() {

    WavFile* wf = new WavFile("/Users/daniel/projects/rack-plugins/ZeroLab/res/sine500.wav");

    wf->load();

    std::cout << "NumSamples: " << wf->numSamples << " 10th sample from the file: " <<
        wf->wavetable[9] << std::endl;
    
    wf->unload();

}