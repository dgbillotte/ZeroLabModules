#include "plugin.hpp"

#include "../dep/dr_wav.h"
#include "lib/Components.hpp"
#include "lib/Grain.hpp"

/*
 * Ideas:
 * - use external input for waveform
 * - allow different envelopes
 * - 
 */

struct ZeroModule : public Module {
	int _downsampleCount = 0;
	int _downsampleRate = 16;
public:
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

struct Grains : ZeroModule {
	enum ParamIds {
		LENGTH_PARAM,
		LENGTH_WIGGLE_PARAM,
		FREQ_PARAM,
		FREQ_WIGGLE_PARAM,
		DENSITY_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		WAVE_OUTPUT,
		ENV_OUTPUT,
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	// params
	int _grainLength = 1000;
	float _grainLengthWiggle = 0.f;
	float _grainFreq = 440.f;
	float _grainFreqWiggle = 0.f;
	float _grainDensity = 0.5f;

	// engine variables
	int _sampleRate = 44100;
	float _cycleLength = 2.f * M_PI;
	// rack::dsp::PulseGenerator _startGrainTrigger;
	int _nextStart = 1;

	// grain variables
	float _gFreq;
	float _gPhase;
	float _gPhaseInc;
	int _gIdx;
	int _gLength;
	Grain* _grain = nullptr;

	float _waveSample;
	float _envSample;

	Grains() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_PARAM, 1.f, 4000.f, 1000.f, "Grain length in samples");
		configParam(LENGTH_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain length wiggle 0-1");
		configParam(FREQ_PARAM, 20.f, 4000.f, 440.f, "Grain frequency in Hz");
		configParam(FREQ_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain frequency wiggle 0-1");
		configParam(DENSITY_PARAM, 0.00001f, 1.f, 0.5f, "Grain density");
	}

	void onSampleRateChange() override;

	void processParams(const ProcessArgs& args) override;
	void processAudio(const ProcessArgs& args) override;

	void _startGrain(float freq, int length);
	float _nextGrainSample();
	float _nextWaveSample();
	float _nextEnvelopeValue();
};

void Grains::onSampleRateChange() {
	_sampleRate = APP->engine->getSampleRate();
}


void Grains::processParams(const ProcessArgs& args) {
	int length = params[LENGTH_PARAM].getValue();
	_grainLength = length;
	float lengthW = params[LENGTH_WIGGLE_PARAM].getValue();
	_grainLengthWiggle = lengthW;
	float freq = params[FREQ_PARAM].getValue();
	_grainFreq = freq;
	float freqW = params[FREQ_WIGGLE_PARAM].getValue();
	_grainFreqWiggle = freqW;
	float density = params[DENSITY_PARAM].getValue();
	_grainDensity = density;
}

void Grains::processAudio(const ProcessArgs& args) {
	if(--_nextStart == 0) {
		// std::cout << "starting grain" << std::endl;
		// _startGrain(_grainFreq, _grainLength);
		_nextStart = _grainLength / _grainDensity;

		if(_grain != nullptr) {
			delete _grain;
		}
		_grain = new Grain(_grainFreq, _grainLength);
	}

	float audioOut = 0.f;
	if(_grain && _grain->running()) {
		audioOut = _grain->nextSample() * 5.f;
	}

	// outputs[WAVE_OUTPUT].setVoltage(_waveSample);
	// outputs[ENV_OUTPUT].setVoltage(_envSample);
	outputs[AUDIO_OUTPUT].setVoltage(audioOut);
}

void Grains::_startGrain(float freq, int length) {
	_gFreq = freq;
	_gLength = length;
	_gPhase = 0.f;
	_gIdx = 0;
}

float Grains::_nextGrainSample() {
	_waveSample = _nextWaveSample();
	_envSample = _nextEnvelopeValue();
	return _waveSample * _envSample;
}

float Grains::_nextWaveSample() {
	float out = sin(_gPhase);

	_gPhaseInc = _cycleLength * _gFreq / _sampleRate;
	_gPhase += _gPhaseInc;
	if(_gPhase >= _cycleLength)
		_gPhase -= _cycleLength;

	// std::cout << "wave val: " << out << std::endl;

	return out *5.f;
}

// create the value at sample fIdx of a simple ramp up/down envelope of length length
float Grains::_nextEnvelopeValue() {
	float mid = _gLength / 2;
	float out = (_gIdx <= mid) ? 
		_gIdx / mid :
		1 - (_gIdx-mid) / mid;
	// std::cout << "env val: " << out << std::endl;
	_gIdx++;
	return out;
}


//------------------------------------------------------------
struct GrainsWidget : ModuleWidget {

	float width = 50.8;
	float midX = width/2;
	float height = 128.5;
    float midY = height/2;
	float _8th = width/8;
	float _7_8th = width-_8th;
    float gutter = 5.f;

	// for 3 columns
	float col1 = width/6;
	float col2 = width/2;
	float col3 = width - col1;	



	GrainsWidget(Grains* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Grains.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Grains::FREQ_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::FREQ_WIGGLE_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::DECAY_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Grains::STRETCH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Grains::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::LENGTH_WIGGLE_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::PICK_POS_PARAM));


		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::DENSITY_PARAM));

		// top row of jacks
		rowY = 87.f;

		// middle row of jacks
		rowY = 100.f;
		// addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, Grains::PLUCK_VOCT_INPUT));
		// addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, Grains::IMPULSE_SAMPLE_INPUT));

		// bottom row of jacks
		rowY = 113.f;
		// addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, Grains::PLUCK_INPUT));
		// addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, Grains::REFRET_INPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col1, rowY)), module, Grains::WAVE_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col2, rowY)), module, Grains::ENV_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, Grains::AUDIO_OUTPUT));
	}
};

Model* modelGrains = createModel<Grains, GrainsWidget>("Grains");