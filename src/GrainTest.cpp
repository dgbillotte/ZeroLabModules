#include <forward_list>

#include "plugin.hpp"
#include "../dep/dr_wav.h"
#include "lib/Components.hpp"
#include "lib/Grain.hpp"
#include "lib/ObjectStore.hpp"
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

	typedef std::function<float(float)> fff;

	fff fSin = [](float x){ return sin(x); };
	WaveSpecLength _sinSpec = WaveSpecLength("SIN_0_2PI_1024", 1024, fSin, 0.f, 2.f*M_PI);

	fff fSin124 = [](float x){ return (sin(x) + sin(2.f * x) + sin(4.f * x)) / 3.f; };
	WaveSpecLength _sin124Spec = WaveSpecLength("SIN(x1,2,4)_0_2PI_1024", 1024, fSin124, 0.f, 2.f*M_PI);

	fff fSin135 = [](float x){ return sin(x)*0.5f + sin(3.f * x)*0.3f + sin(5.f * x)*0.2f; };
	WaveSpecLength _sin135Spec = WaveSpecLength("SIN(x1,3,5)_0_2PI_1024", 1024, fSin135, 0.f, 2.f*M_PI);

	fff fSqr = [](float x) { return (x < 5.f) ? 1.f : -1.f; };
	WaveSpecLength _sqrSpec = WaveSpecLength("SQR_0_10_10", 10, fSqr, 0.f, 10.f);

	fff fSaw = [](float x){ return (x * 2.f/9.f) - 1.f; };
	WaveSpecLength _sawSpec = WaveSpecLength("SAW_0_10_10", 10, fSaw, 0.f, 10.f);

	fff fCos = [](float x) { return (cos(x) + 1.f) / 2.f; };
	LUTSpec _cosSpec = LUTSpec("COS_0_2PI_1024", 1024, fCos, -M_PI, M_PI);

	fff fSinc = [](float x) { float t=x*M_PI; return sin(t)/t; };
	LUTSpec _sinc2Spec = LUTSpec("SINC_-2_2_1024", 1024, fSinc, -2.f, 2.f);
	LUTSpec _sinc3Spec = LUTSpec("SINC_-3_3_1024", 1024, fSinc, -3.f, 3.f);
	LUTSpec _sinc4Spec = LUTSpec("SINC_-4_4_1024", 1024, fSinc, -4.f, 4.f);
	LUTSpec _sinc5Spec = LUTSpec("SINC_-5_5_1024", 1024, fSinc, -5.f, 5.f);
	LUTSpec _sinc6Spec = LUTSpec("SINC_-6_6_1024", 1024, fSinc, -6.f, 6.f);



	// engine variables
	int _sampleRate = 44100;
	int _nextStart = 1;

	typedef std::shared_ptr<Grain> GrainPtr;
	GrainPtr _grain;// = GrainPtr(new Grain(100,10));
	WaveTablePtr _wavetable;
	LUTPtr _lut;

	GrainTest() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_PARAM, 10.f, 4000.f, 1000.f, "Grain length in samples");
		configParam(LENGTH_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain length wiggle 0-1");
		configParam(FREQ_PARAM, 20.f, 4000.f, 440.f, "Grain frequency in Hz");
		configParam(FREQ_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain frequency wiggle 0-1");
		configParam(DENSITY_PARAM, 0.3f, 10.f, 0.5f, "Grains per second");
		configParam(DENSITY_WIGGLE_PARAM, 0.f, 1.f, 0.f, "Grain density wiggle 0-1");
		configParam(RAMP_PCT_PARAM, 0.01f, 0.5f, 0.2f, "Ramp Length");
		configParam(RAMP_TYPE_PARAM, Grain::ENV_PSDO_GAUSS, Grain::NUM_ENV_TYPES-0.01, Grain::ENV_RAMP, "Ramp Type");
		configParam(WAVE_TYPE_PARAM, Grain::WAV_SIN, Grain::NUM_WAV_TYPES-0.01, Grain::WAV_SIN, "Wave Type");

		setWaveform();
		setEnvelope();

		_grain = GrainPtr(new Grain(_newOsc(), _newEnv()));
	}


	void onSampleRateChange() override;

	void processParams(const ProcessArgs& args) override;
	void processAudio(const ProcessArgs& args) override;
	float _wiggle(float in, float wiggle);

	void setWaveform() {
		auto waveBank = ObjectStore::getStore();
		// WaveTablePtr wavetable;
			// _wavetable = waveBank->loadWavetable(_sinSpec);
        if(_waveType == Grain::WAV_SIN) {
			_wavetable = waveBank->loadWavetable(_sinSpec);
        } else if(_waveType == Grain::WAV_SQR) {
			_wavetable = waveBank->loadWavetable(_sqrSpec);
        } else if(_waveType == Grain::WAV_SAW) {
			_wavetable = waveBank->loadWavetable(_sawSpec);
        } else if(_waveType == Grain::WAV_SIN1_3_5) {
			_wavetable = waveBank->loadWavetable(_sin135Spec);
        } else { //if(_waveType == Grain::WAV_SIN1_2_4) {
			_wavetable = waveBank->loadWavetable(_sin124Spec);
        }
	}

	void setEnvelope() {
		auto waveBank = ObjectStore::getStore();


        if(_rampType == Grain::ENV_PSDO_GAUSS) {
			_lut = waveBank->loadLUT(_cosSpec);
        } else if(_rampType == Grain::ENV_SINC2) {
			_lut = waveBank->loadLUT(_sinc2Spec);
        } else if(_rampType == Grain::ENV_SINC3) {
			_lut = waveBank->loadLUT(_sinc3Spec);
        } else if(_rampType == Grain::ENV_SINC4) {
			_lut = waveBank->loadLUT(_sinc4Spec);
        } else if(_rampType == Grain::ENV_SINC5) {
			_lut = waveBank->loadLUT(_sinc5Spec);
        } else if(_rampType == Grain::ENV_SINC6) {
			_lut = waveBank->loadLUT(_sinc6Spec);
        }
	}

	WTFOscPtr _newOsc() {
		return WTFOscPtr(new WTFOsc(_wavetable, _wiggle(_grainFreq, _grainFreqWiggle), APP->engine->getSampleRate()));
	}

	LUTEnvelopePtr _newEnv() {
		return LUTEnvelopePtr(new LUTEnvelope(_lut, _wiggle(_grainLength, _grainLengthWiggle), APP->engine->getSampleRate(), _rampLength));
	}
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

	int rampType = params[RAMP_TYPE_PARAM].getValue();
	if(rampType != _rampType) {
		_rampType = rampType;
		setEnvelope();
	}
	
	float waveType = params[WAVE_TYPE_PARAM].getValue();
	if(waveType != _waveType) {
		_waveType = waveType;
		setWaveform();
	}

}

inline float GrainTest::_wiggle(float in, float wiggle) {
    if(wiggle == 0.f)
        return in;
	float wrand = (2 * (float)rand() / (float)RAND_MAX) - 1; // ends up in [-1..1]
	return in + in * wrand * wiggle; // in +/- in*wiggle
}

void GrainTest::processAudio(const ProcessArgs& args) {
    if(--_nextStart <= 0) {

		auto osc = _newOsc();
		auto env = _newEnv();
		_grain = GrainPtr(new Grain(osc, env, args.sampleRate));

		_nextStart = args.sampleRate / _wiggle(_grainDensity, _grainDensityWiggle);
    }

	float audioOut = 0.f;
    float envOut = 0.f;
    float waveOut = 0.f;
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