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
### AnaLogic2.cpp
analogic is signal combiner that takes two input signals and then combines the two signals in a myriad of ways producing 4
independent outputs and one mix output. The first part of the system provides for the shifting, scaling, and inverting of the
input signal and provides outputs for these pre-processed signals. In this fashion it can be used as a 2 channel signal scaler/shifter/inverter.

The combination logic is based off of the idea of boolean logic as applied to digital signals and how that system would react to audio signals. I came up with several different ways to models the boolean operations of *and* and *or* for digital signals. The large "logic?" knob chooses between the 4 logic models and smoothly morphs between them.

The signals, which could be digital (0-10v) or analog (+/- 5V) are run through the logic operators and the output is captured as the "analog output" for each operator. The analog outputs are then digitized into rectangle waves which are output as the "digital outputs".

The final stage of the module is a mixer which combines the 4 signals according the the parameters: earth, water, fire. I'll let you figure out what each of them does.

A lot of the inspiration for this module came from hearing Eli Pechman, of [Mystic Circuits](https://www.mysticcircuits.com/), describe his module "Ana" on the [Podular Modcast](https://podularmodcast.fireside.fm/) podcast and from building his Spectra Mirror module. Thanks Eli!

### Strings.cpp
Strings is an implementation of the Karplus-Strong vibrating string models. My work is based off of what I learned in BasicSynth, as well as reading the original paper by Karplus & Strong as well as the paper by Jaffe & Smith which describes several extensions to the original Karplus-Strong models. At this point I have experimented with some things that haven't worked and am concentrating on the Jaffe & Smith extensions.

The top knob is the frequency for the pluck. The 2nd knob down is the low pass filter. The 3rd knob and the switch currently do nothing.

The top input takes a trigger/gate to pluck the model. The second input is a 1V/Oct input. The third output is the module's audio output.

### SpectralMix.cpp
The idea of this module is to allow you to separate a input given signal into 3 different spectral bands, process those bands independently, and then mix it all back together for the output.

The top three knobs control the cutoff frequencies for each band: Lo, Mid, High.

The next section (2 inputs, 1 knob, 1 output) are the send/receive for each channel. The first jack is the send, the knob mixes the return with the original signal. The next jack is the return input and the last jack is CV control for the mix.

The last section is the mix-out section controlling how much of each band is in the final signal out. The jacks under the knobs are CV controls for them.

### ADSR.cpp
This is one of the earlier modules I "built", but I use it often. It is the Fundamental ADSR module that I just modified by adding outputs gates for each section of the envelope.

### Resonator.cpp
A very simple resonator implementation. I don't find it that interesting, yet..., but it works

### Some others that work, kinda...
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
