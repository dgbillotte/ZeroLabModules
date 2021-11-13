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
 * - grainTest producing 0.0000 output shouldn't be counted in total
 */

struct GrainTest : ZeroModule {
	enum ParamIds {
		LENGTH_PARAM,
		LENGTH_WIGGLE_PARAM,
		FREQ_PARAM,
		FREQ_WIGGLE_PARAM,
		DENSITY_PARAM,
		DENSITY_WIGGLE_PARAM,
        RAMP_PCT_PARAM,
		RAMP_TYPE_PARAM,
		WAVE_TYPE_PARAM,
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
    float _rampLength = 0.2f;
	int _rampType = 0;
	int _waveType = 0;

	// engine variables
	int _sampleRate = 44100;
	int _nextStart = 1;

	typedef std::shared_ptr<Grain> GrainPtr;
	GrainPtr _grain = GrainPtr(new Grain(100,10));

	GrainTest() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_PARAM, 10.f, 4000.f, 1000.f, "Grain length in samples");
		configParam(LENGTH_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain length wiggle 0-1");
		configParam(FREQ_PARAM, 20.f, 4000.f, 440.f, "Grain frequency in Hz");
		configParam(FREQ_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain frequency wiggle 0-1");
		configParam(DENSITY_PARAM, 0.3f, 10.f, 0.5f, "Grains per second");
		configParam(DENSITY_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain density wiggle 0-1");
		configParam(RAMP_PCT_PARAM, 0.01f, 0.5f, 0.2f, "Ramp Length");
		configParam(RAMP_TYPE_PARAM, Grain::ENV_PSDO_GAUSS, Grain::ENV_RAMP-0.01, Grain::ENV_RAMP, "Ramp Type");
		configParam(WAVE_TYPE_PARAM, Grain::WAV_SIN, Grain::WAV_SAW-0.01, Grain::WAV_SIN, "Wave Type");
	}

	void onSampleRateChange() override;

	void processParams(const ProcessArgs& args) override;
	void processAudio(const ProcessArgs& args) override;
	float _wiggle(float in, float wiggle);
};


void GrainTest::onSampleRateChange() {
	// _sampleRate = APP->engine->getSampleRate();
}

void GrainTest::processParams(const ProcessArgs& args) {
	_grainLength = params[LENGTH_PARAM].getValue();
	_grainLengthWiggle = params[LENGTH_WIGGLE_PARAM].getValue();
	_grainFreq = params[FREQ_PARAM].getValue();
	_grainFreqWiggle = params[FREQ_WIGGLE_PARAM].getValue();
	_grainDensity = params[DENSITY_PARAM].getValue();
	_grainDensityWiggle = params[DENSITY_WIGGLE_PARAM].getValue();
	_rampLength = params[RAMP_PCT_PARAM].getValue();
	_rampType = params[RAMP_TYPE_PARAM].getValue();
	_waveType = params[WAVE_TYPE_PARAM].getValue();
}

inline float GrainTest::_wiggle(float in, float wiggle) {
    if(wiggle == 0.f)
        return in;
	float wrand = (2 * (float)rand() / (float)RAND_MAX) - 1; // ends up in [-1..1]
	return in + in * wrand * wiggle; // in +/- in*wiggle
}

void GrainTest::processAudio(const ProcessArgs& args) {
	float audioOut = 0.f;
    float envOut = 0.f;
    float waveOut = 0.f;
    
    if(--_nextStart <= 0) {
		auto g = new Grain(_wiggle(_grainFreq, _grainFreqWiggle),
			_wiggle(_grainLength, _grainLengthWiggle), args.sampleRate, _rampLength, _rampType, _waveType);
		_grain = GrainPtr(g);

		_nextStart = args.sampleRate / _wiggle(_grainDensity, _grainDensityWiggle);
    }

    if(_grain->running()) {
        audioOut = _grain->nextSample() * 5.f;
        envOut = _grain->envOut();
        waveOut = _grain->wavOut();
	}
	
	outputs[AUDIO_OUTPUT].setVoltage(audioOut);
	outputs[ENV_OUTPUT].setVoltage(envOut);
	outputs[WAVE_OUTPUT].setVoltage(waveOut);
}


//------------------------------------------------------------
struct GrainTestWidget : ModuleWidget {

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




	GrainTestWidget(GrainTest* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GrainTest.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainTest::FREQ_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, GrainTest::FREQ_WIGGLE_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainTest::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, GrainTest::LENGTH_WIGGLE_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainTest::DENSITY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, GrainTest::DENSITY_WIGGLE_PARAM));

		rowY = 69.f;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, GrainTest::WAVE_TYPE_PARAM));

		// top row of jacks
		rowY = 87.f;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainTest::RAMP_PCT_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, GrainTest::RAMP_TYPE_PARAM));

		// middle row of jacks
		rowY = 100.f;

		// bottom row of jacks
		rowY = 113.f;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col1, rowY)), module, GrainTest::WAVE_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col2, rowY)), module, GrainTest::ENV_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, GrainTest::AUDIO_OUTPUT));
	}
};

Model* modelGrainTest = createModel<GrainTest, GrainTestWidget>("GrainTest");