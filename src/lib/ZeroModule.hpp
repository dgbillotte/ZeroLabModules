#ifndef ZERO_MODULE_HPP
#define ZERO_MODULE_HPP

struct ZeroModule : public Module {
	int _downsampleCount = 0;
	int _downsampleRate = 16;

	void setDownsampleRate(int downsampleRate) {
		_downsampleRate = downsampleRate;
	}
	void process(const ProcessArgs& args) override {
		if(_downsampleCount++ == _downsampleRate) {
			_downsampleCount = 0;
			processParams(args);
		}

		processAudio(args);
	}

	virtual void processParams(const ProcessArgs& args) {
		return;
	};

	virtual void processAudio(const ProcessArgs& args) = 0;
};

#endif
