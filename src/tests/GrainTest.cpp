#include <iostream>
#include "Grain.hpp"
// #include "AllPassFilter.hpp"
// #include <assert.h>

int DUMP = true;
// #include "dump.hpp"

int main() {

    Grain g(440, 50);

    int count = 1;

    while(g.running()) {
        float audio = g.nextSample();
        float env = g.envOut();
        float wav = g.wavOut();
        std::cout << count << ", env: " << env << ", wav: " << wav <<
             ", out: " << audio << std::endl;
        count++;
    }
    g.dump();

    // for(int i=0; i < cosLUT.size(); i++) {
    //     std::cout << "cos(" << i << ") = " << cosLUT.at(i)<< std::endl;
    //     // std::cout << "cos(" << i+0.5f << ") = " << cosLUT.atInterp(i+0.5f)<< std::endl;

    // }


    return 0;

}
