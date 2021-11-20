#include <forward_list>

#include "plugin.hpp"
#include "../dep/dr_wav.h"
#include "lib/Components.hpp"
#include "lib/Grain.hpp"
#include "lib/RGrain.hpp"
#include "lib/ObjectStore.hpp"
#include "lib/ZeroModule.hpp"

/*
 * Ideas:
 * - use external input for waveform
 * - allow different envelopes
 * - max amplitude (< 1) for envelope
 * - make repeatable
 * - grainTest producing 0.0000 output shouldn't be counted in total
 * 
 * Notes:
 * - I don't fully understand why, but as of now, _wavetable and _lut must
 *   be class members. I tried to make them local in setWaveform and
 *   setEnvelope, but it causes strange crashes. I would think that the
 *   smart pointer would handle this correctly, so I must be looking
 *   at it wrong.
 * - This just occurred to me, but in making major changes to a module,
 *   sometimes it just won't run right as if it is holding on to past
 *   state. If you change any of the parameter/IO enums RACK could be
 *   holding onto no-longer valid values when you start back up.
 * - Really need to figure out making a modulel start back up, as in a 
 *   saved-patch and set all of its values properly
 */

struct GrainPulse : ZeroModule {
	enum ParamIds {
		LENGTH_PARAM,
		FREQ_PARAM,
		DELAY_PARAM,
        RAMP_PCT_PARAM,
		RAMP_TYPE_PARAM,
		WAVE_TYPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
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
	float _grainDensity = 0.5f;
    float _rampLength = 0.2f;
	int _rampType = 0;
	int _waveType = 0;

    enum WaveTypes {
        WAV_SIN,
        WAV_SIN1_3_5,
        WAV_SIN1_2_4,
        WAV_SQR,
        WAV_SAW,
        NUM_WAV_TYPES
    };

	enum EnvelopeTypes {
        ENV_PSDO_GAUSS,
        ENV_SINC2,
        ENV_SINC3,
        ENV_SINC4,
        ENV_SINC5,
        ENV_SINC6,
        ENV_RAMP,
		NUM_ENV_TYPES
	};


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
	bool _useExternalWave = false;

	typedef std::shared_ptr<Grain> GrainPtr;
	GrainPtr _grain;
	GrainPtr _thruGrain;
	WaveTablePtr _wavetable;
	LUTPtr _lut;
	WTFOsc _osc;
	LUTEnvelope _env;
	ThruOsc _extOsc;

	ObjectStorePtr _waveBank;

	GrainPulse() :
		_grain(new Grain(_osc, _env, 100)),
		_thruGrain(new Grain(_extOsc, _env, 100))
	{
		// configure this module
		setDownsampleRate(32);
		timingOn(false);


		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_PARAM, 10.f, 4000.f, 1000.f, "Grain length in samples");
		configParam(FREQ_PARAM, 20.f, 4000.f, 440.f, "Grain frequency in Hz");
		configParam(DELAY_PARAM, 0.f, 10000.f, 0.f, "Delay between grain repeats in samples");
		configParam(RAMP_PCT_PARAM, 0.01f, 0.5f, 0.2f, "Ramp Length");
		configParam(RAMP_TYPE_PARAM, ENV_PSDO_GAUSS, NUM_ENV_TYPES-0.01, ENV_RAMP, "Ramp Type");
		configParam(WAVE_TYPE_PARAM, WAV_SIN, NUM_WAV_TYPES-0.01, WAV_SIN, "Wave Type");

		_waveBank = ObjectStore::getStore();

		setWaveform();
		_osc.sampleRate(APP->engine->getSampleRate());
		_osc.freq(200);

		setEnvelope();
		_env.length(100);
		_env.envRampLength(0.2f);

	}

	void processParams(const ProcessArgs& args) override;
	void processAudio(const ProcessArgs& args) override;

	void setWaveform() {
        if(_waveType == WAV_SIN) {
			_wavetable = _waveBank->loadWavetable(_sinSpec);
        } else if(_waveType == WAV_SQR) {
			_wavetable = _waveBank->loadWavetable(_sqrSpec);
        } else if(_waveType == WAV_SAW) {
			_wavetable = _waveBank->loadWavetable(_sawSpec);
        } else if(_waveType == WAV_SIN1_3_5) {
			_wavetable = _waveBank->loadWavetable(_sin135Spec);
        } else { //if(_waveType == WAV_SIN1_2_4) {
			_wavetable = _waveBank->loadWavetable(_sin124Spec);
        }

		_osc.wavetable(_wavetable);
	}

	void setEnvelope() {
        if(_rampType == ENV_PSDO_GAUSS) {
			_lut = _waveBank->loadLUT(_cosSpec);
        } else if(_rampType == ENV_SINC2) {
			_lut = _waveBank->loadLUT(_sinc2Spec);
        } else if(_rampType == ENV_SINC3) {
			_lut = _waveBank->loadLUT(_sinc3Spec);
        } else if(_rampType == ENV_SINC4) {
			_lut = _waveBank->loadLUT(_sinc4Spec);
        } else if(_rampType == ENV_SINC5) {
			_lut = _waveBank->loadLUT(_sinc5Spec);
        } else if(_rampType == ENV_SINC6) {
			_lut = _waveBank->loadLUT(_sinc6Spec);
        }

		_env.lut(_lut);
	}
};


void GrainPulse::processParams(const ProcessArgs& args) {
	// can be set directly with no problems
	_osc.freq(params[FREQ_PARAM].getValue());

	size_t repeatDelay = params[DELAY_PARAM].getValue();
	_grain->repeatDelay(repeatDelay);
	_thruGrain->repeatDelay(repeatDelay);

	// envelope and waveform should only be set when changed
	int rampType = params[RAMP_TYPE_PARAM].getValue();
	if(_rampType != rampType) {
		_rampType = rampType;
		setEnvelope();
	}
	
	float waveType = params[WAVE_TYPE_PARAM].getValue();
	if(_waveType != waveType) {
		_waveType = waveType;
		setWaveform();
	}

	// these two may be able to be directly set
	float rampLength = params[RAMP_PCT_PARAM].getValue();
	bool updateEnvRampLen = false;
	if(_rampLength != rampLength) {
		_rampLength = rampLength;
		updateEnvRampLen = true;
	}

	int grainLength = params[LENGTH_PARAM].getValue();
	if(_grainLength != grainLength || updateEnvRampLen) {
		_grainLength = grainLength;
		_env.length(_grainLength);
		_env.envRampLength(_rampLength);
	}

	_useExternalWave = inputs[AUDIO_INPUT].isConnected();
}

void GrainPulse::processAudio(const ProcessArgs& args) {
	// run the grain engine
	float audioOut = 0.f;
    float envOut = 0.f;
    float waveOut = 0.f;
	if(_useExternalWave) {
		audioOut = _thruGrain->nextSample() * 5.f;
		envOut = _thruGrain->envOut();
		waveOut = _thruGrain->wavOut();

	} else {
		audioOut = _grain->nextSample() * 5.f;
		envOut = _grain->envOut();
		waveOut = _grain->wavOut();
	}

	float input = inputs[AUDIO_INPUT].getVoltage() / 5.f;
	_extOsc.setNext(input);
	
	outputs[AUDIO_OUTPUT].setVoltage(audioOut);
	outputs[ENV_OUTPUT].setVoltage(envOut);
	outputs[WAVE_OUTPUT].setVoltage(waveOut);
}


//------------------------------------------------------------
struct GrainPulseWidget : ModuleWidget {

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




	GrainPulseWidget(GrainPulse* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GrainPulse.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainPulse::FREQ_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainPulse::LENGTH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainPulse::DELAY_PARAM));

		rowY = 69.f;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, GrainPulse::WAVE_TYPE_PARAM));

		// top row of jacks
		rowY = 87.f;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, GrainPulse::RAMP_PCT_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, GrainPulse::RAMP_TYPE_PARAM));

		// middle row of jacks
		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, GrainPulse::AUDIO_INPUT));

		// bottom row of jacks
		rowY = 113.f;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col1, rowY)), module, GrainPulse::WAVE_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col2, rowY)), module, GrainPulse::ENV_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, GrainPulse::AUDIO_OUTPUT));
	}
};

Model* modelGrainPulse = createModel<GrainPulse, GrainPulseWidget>("GrainPulse");