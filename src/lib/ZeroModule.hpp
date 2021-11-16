#ifndef ZERO_MODULE_HPP
#define ZERO_MODULE_HPP

struct ZeroModule : public Module {
	int _downsampleCount = 0;
	int _downsampleRate = 16;
	bool _debugOn = true;

	void setDownsampleRate(int downsampleRate) {
		_downsampleRate = downsampleRate;
	}
	void process(const ProcessArgs& args) override {
		if(_downsampleCount++ == _downsampleRate) {
			_downsampleCount = 0;
			processParams(args);
		}

		// if(_debugOn) {
		// 	try {
		// 		processAudio(args);
		// 	}
		// 	catch (const std::out_of_range& oor) {
		// 		std::cerr << "Out of Range error in processAudio(): " << oor.what() << '\n';
		// 	}

		// } else {
			processAudio(args);
		// }

	}

	virtual void processParams(const ProcessArgs& args) {
		return;
	};

	virtual void processAudio(const ProcessArgs& args) = 0;
};

#endif
