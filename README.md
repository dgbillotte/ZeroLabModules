# ZeroLabModules
This is currently my lab/sandbox for continued learning about [VCV-Rack](https://vcvrack.com/) and DSP programming.

Inside you will find a collection of experimental modules and library components of varying degrees of completion.
Some of them might make good code reading, some of them, not so much. I'm sure you can find some off by one bugs...

The majority of my learning about DSP has been from the following books and I've ***tried*** to give credit as due at the
top of each code file.
- [BasicSynth](http://basicsynth.com/) by Daniel R. Mitchell. 
- [Designing Audio Effect Plug-Ins in C++](https://www.willpirkle.com/about/books/) by [Will Pirkle](http://www.willpirkle.com/)
- [Designing Software Synthesizer Plug-Ins in C++](https://www.willpirkle.com/about/books/) by [Will Pirkle](http://www.willpirkle.com/)

#### Notes on BasicSynth
I have to say that I don't cite this book often because it was
the first book I read on the topic and my earlier experiements aren't present here to cite, so I have to say: I found this book
friendly and "easy" (DSP-easy that is) to get started with and all of the DSP code that I write now is somehow influenced by it.

#### Notes on Will Pirkle's Books
While BasicSynth is a good start into DSP, Will Pirkle takes you deep into the depths of the topic with clear writing, useful diagrams,
charts, graphs, etc, and real code and projects. He has a number of audio development tools available that look really useful, but I haven't
played with them yet. NOTE: I have the first editions of his books (linked above) and the [2nd editions](https://www.amazon.com/s?i=stripbooks&rh=p_27%3AWill+Pirkle) look to be updated quite a bit so I'm
sure that they are even better.

## Modules (that have some kind of purpose and basically work)
- ADSR.cpp (this is the Fundamental module with gate outputs for each stage)
- AnaLogic[2].cpp : While I didn't intend to emulate Mystic Circuit's Ana, as I was writing this I think I figured out Eli's inspiration for the name
- Resonator.cpp
- SimpleDelay.cpp
- TrackHold.cpp : inspired by Eli's (Mystic Circuits) description of how the Spectra Mirror worked, a sample/track and hold


## Library Code (that basically works)
These are "useable" but not thouroughly hardened or tested and are all a work in progress
- DelayBuffer.hpp : an over-dub-able circular buffer meant specifically for delays. Simple and fast
- Osc.hpp : framework for creating oscillators that have a period of 2π. This is really meant more for being able to easily experiment with different waveform generation methods than to create really fast/efficient "production" oscillators. Inspired by both Mitchell and Pirkle.
- Filters: These are all implementations of algorithms from Pirkle or inspired by
  -  AllPassFilter.hpp: Simple all-pass filter with variable D and g
  -  CombFilter.hpp : Simple comb filter with variable D and g
  -  Filter.hpp : A collection of filter types including a generic bi-quad
