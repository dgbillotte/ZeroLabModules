#include <forward_list>

#include "plugin.hpp"
#include "../dep/dr_wav.h"
#include "lib/Components.hpp"
#include "lib/Grain.hpp"
#include "lib/ZeroModule.hpp"

/*
 * Ideas:
 * - use external input for waveform
 * - allow different envelopes
 * - max amplitude (< 1) for envelope
 * - make repeatable
 * - grains producing 0.0000 output shouldn't be counted in total
 */

struct Grains : ZeroModule {
	enum ParamIds {
		LENGTH_PARAM,
		LENGTH_WIGGLE_PARAM,
		FREQ_PARAM,
		FREQ_WIGGLE_PARAM,
		DENSITY_PARAM,
		DENSITY_WIGGLE_PARAM,
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
	float _grainDensityWiggle = 0.f;

	// engine variables
	int _sampleRate = 44100;
	int _nextStart = 1;

	typedef std::shared_ptr<Grain> GrainPtr;
	std::list<GrainPtr> _grains;

	Grains() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_PARAM, 10.f, 4000.f, 1000.f, "Grain length in samples");
		configParam(LENGTH_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain length wiggle 0-1");
		configParam(FREQ_PARAM, 20.f, 4000.f, 440.f, "Grain frequency in Hz");
		configParam(FREQ_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain frequency wiggle 0-1");
		configParam(DENSITY_PARAM, 0.01f, 10.f, 0.5f, "Grains per second");
		configParam(DENSITY_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain density wiggle 0-1");
	}

	void onSampleRateChange() override;

	void processParams(const ProcessArgs& args) override;
	void processAudio(const ProcessArgs& args) override;
	float _wiggle(float in, float wiggle);
};


void Grains::onSampleRateChange() {
	// _sampleRate = APP->engine->getSampleRate();
}

void Grains::processParams(const ProcessArgs& args) {
	_grainLength = params[LENGTH_PARAM].getValue();
	_grainLengthWiggle = params[LENGTH_WIGGLE_PARAM].getValue();
	_grainFreq = params[FREQ_PARAM].getValue();
	_grainFreqWiggle = params[FREQ_WIGGLE_PARAM].getValue();
	_grainDensity = params[DENSITY_PARAM].getValue();
	_grainDensityWiggle = params[DENSITY_WIGGLE_PARAM].getValue();
}

inline float Grains::_wiggle(float in, float wiggle) {
	float wrand = (2 * (float)rand() / (float)RAND_MAX) - 1; // ends up in [-1..1]
	return in + in * wrand * wiggle; // in +/- in*wiggle
}

void Grains::processAudio(const ProcessArgs& args) {
	if(--_nextStart == 0) {
		GrainPtr g = GrainPtr(new Grain(_wiggle(_grainFreq, _grainFreqWiggle),
			_wiggle(_grainLength, _grainLengthWiggle), args.sampleRate));
		_grains.push_back(g);
		
		_nextStart = _grainLength / _wiggle(_grainDensity, _grainDensityWiggle);
	}

	float audioOut = 0.f;
	int numGrains = 0;

	std::list<GrainPtr>::iterator it = _grains.begin();
	while(it != _grains.end()) {
		GrainPtr g = *it;
		if(g->running()) {
			++it;
			audioOut += g->nextSample();
			numGrains++;
		} else {
			it = _grains.erase(it);
		}
	}

	audioOut = (numGrains > 0) ? 5.f * audioOut / numGrains : 0.f;
	outputs[AUDIO_OUTPUT].setVoltage(audioOut);
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

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Grains::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::LENGTH_WIGGLE_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Grains::DENSITY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Grains::DENSITY_WIGGLE_PARAM));

		// top row of jacks
		rowY = 87.f;

		// middle row of jacks
		rowY = 100.f;

		// bottom row of jacks
		rowY = 113.f;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col1, rowY)), module, Grains::WAVE_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col2, rowY)), module, Grains::ENV_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, Grains::AUDIO_OUTPUT));
	}
};

Model* modelGrains = createModel<Grains, GrainsWidget>("Grains");