#include <iostream>
#define TESTING 
#include "Grain.hpp"
#include "ObjectStore.hpp"
// #include "AllPassFilter.hpp"
// #include <assert.h>

int DUMP = true;
// #include "dump.hpp"

LUT cosLUT = LUT(0.f, 2.f*M_PI, 2000, cos);
WaveTable sinWT(0.f, 2.f*M_PI, 100, sin);

int main() {

    // Grain g(440, 100);

    // int count = 1;

    // while(g.running()) {
    //     float audio = g.nextSample();
    //     float env = g.envOut();
    //     float wav = g.wavOut();
    //     std::cout << count << ", env: " << env << ", wav: " << wav <<
    //          ", out: " << audio << std::endl;
    //     count++;
    // }
    // g.dump();

    // for(size_t i=0; i < cosLUT.size(); i++) {
    //     std::cout << "cosIdx(" << i << ") = " << cosLUT.atIdx(i)<< std::endl;
    //     // float theta = i*2.f*M_PI/(cosLUT.size());
    //     // std::cout << "cos(" << theta << ") = " << cosLUT.at(theta) << std::endl;

    // }

    for(size_t i=0; i < sinWT.size(); i++) {
        std::cout << "sinIdx(" << i << ") = " << sinWT.atIdx(i)<< std::endl;
        // float theta = i*2.f*M_PI/(sinWT.size());
        std::cout << "sin(" << i+0.1f << ") = " << sinWT.at(i+0.1f) << std::endl;
    }


    return 0;

}
