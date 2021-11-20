#ifndef ZERO_MODULE_HPP
#define ZERO_MODULE_HPP

#include <chrono>
using namespace std::chrono;
struct ZeroModule : public Module {
	int _downsampleCount = 0;
	int _downsampleRate = 16;
	// bool _debugOn = false;
	bool _timingOn = false;

	void setDownsampleRate(int downsampleRate) {
		_downsampleRate = downsampleRate;
	}

	// void debugOn(bool debugOn) { _debugOn = debugOn; }
	void timingOn(bool timingOn) { _timingOn = timingOn; }

	size_t testTime = 0;
	int numTests = 44100*5;
	int testCount = 0;

	void process(const ProcessArgs& args) override {
		if(_downsampleCount++ == _downsampleRate) {
			_downsampleCount = 0;
			processParams(args);
		}

		if(_timingOn) {
			auto start = high_resolution_clock::now();

			processAudio(args);

			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			testTime += duration.count();
			if(++testCount > numTests) {
				float ave = (float)testTime/(float)testCount;
				std::cout << "avg sample gen time: " << ave << " 𝝻s" << std::endl;
				testTime = 0;
				testCount = 0;
			}			
		} else {
			processAudio(args);
		}
	}

	virtual void processParams(const ProcessArgs& args) {
		return;
	};

	virtual void processAudio(const ProcessArgs& args) = 0;
};

#endif
