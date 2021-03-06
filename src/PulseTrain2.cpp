#include <list>

#include "plugin.hpp"
#include "../dep/dr_wav.h"
#include "lib/Components.hpp"
#include "lib/Pulsar.hpp"
#include "lib/PulsarTrain.hpp"
#include "lib/ObjectStore.hpp"
#include "lib/ZeroModule.hpp"

/*
 * Ideas:

 * - allow different envelopes
 * - max amplitude (< 1) for envelope
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
 * - Really need to figure out making a module start back up, as in a 
 *   saved-patch and set all of its values properly
 */

struct PulseTrain2 : ZeroModule {
	enum ParamIds {
		LENGTH_PARAM,
		DUTY_PARAM,
		FREQ_PARAM,
        RAMP_PCT_PARAM,
		RAMP_TYPE_PARAM,
		WAVE_TYPE_PARAM,
        TRAIN_LENGTH_PARAM,
        TRAIN_RAMP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
        TRIGGER_INPUT,
		VOCT_INPUT,
        LENGTH_CV_INPUT,
        DUTY_CV_INPUT,
        ENV_RAMP_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TRIGGER_OUTPUT,
		WAVE_OUTPUT,
		ENV_OUTPUT,
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	// params
	int _pulsarLength = 1000;
	float _pulsarDuty = 0.5f;
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

	bool _useExternalWave = false;
	WaveTablePtr _wavetable;
	LUTPtr _lut;
	WTFOscPtr _osc;
	ThruOscPtr _extOsc;
	LUTEnvelopePtr _env;
	float _freq = 20.f;
	// typedef std::shared_ptr<Pulsar> PulsarPtr;
	PulsarPtr _pulsar;
	std::list<PulsarTrainPtr> _pulsars;
	ObjectStorePtr _waveBank;
    rack::dsp::PulseGenerator _nextPulse;

	// stuff for trigger trains
    dsp::SchmittTrigger _trainTrig;
    size_t _trainLength;
    float _trainAttack;
    size_t _trainIdx = 0;

	// book-keeping for the train envelope
    size_t _trainEnvKnee = 0;
    float _trainEnvVal = 0.f;
    float _trainEnvUpInc = 0.f;
    float _trainEnvDownInc = 0.f;

	PulseTrain2() :
        _osc(WTFOscPtr(new WTFOsc())),
        _extOsc(ThruOscPtr(new ThruOsc())),
        _env(LUTEnvelopePtr(new LUTEnvelope())),
		_pulsar(new Pulsar(_osc, _env, 2000, 0.5f)),
        _waveBank(ObjectStore::getStore())
	{
		// configure this module
		setDownsampleRate(32);
		timingOn(false);

        // configure params
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_PARAM, 10.f, 8000.f, 1000.f, "Pulsar length in samples");
		configParam(FREQ_PARAM, 20.f, 4000.f, 440.f, "Pulsar frequency in Hz");
		configParam(DUTY_PARAM, 0.f, 1.f, 0.5f, "Pusar duty-cycle: 0 to 1");
		configParam(RAMP_PCT_PARAM, 0.f, 0.5f, 0.2f, "Ramp Length");
		configParam(RAMP_TYPE_PARAM, ENV_PSDO_GAUSS, NUM_ENV_TYPES-0.01, ENV_RAMP, "Ramp Type");
		configParam(WAVE_TYPE_PARAM, WAV_SIN, NUM_WAV_TYPES-0.01, WAV_SIN, "Wave Type");
		configParam(TRAIN_LENGTH_PARAM, 100.f, 50000.f, 10000.f, "Pulsar-Train length in samples");
		configParam(TRAIN_RAMP_PARAM, 0.f, 1.f, 0.f, "Pulsar-Train Ramp thing");

        // setup the oscillator and envelope
		setWaveform();
		_osc->sampleRate(APP->engine->getSampleRate());
		_osc->freq(200);

		setEnvelope();
		_env->length(100);
		_env->envRampLength(0.2f);

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

		_osc->wavetable(_wavetable);
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

		_env->lut(_lut);
	}
};

size_t pulsarLength;
float _trainDuty;

void PulseTrain2::processParams(const ProcessArgs& args) {
    // main Pulsar params
    size_t length = params[LENGTH_PARAM].getValue();
    if(inputs[LENGTH_CV_INPUT].isConnected()){
        length += inputs[LENGTH_CV_INPUT].getVoltage() * 100.f;
    }
	_pulsarLength = length;
    _pulsar->p(clamp(length, 10, 10000));
    
    float duty = params[DUTY_PARAM].getValue();
    if(inputs[DUTY_CV_INPUT].isConnected()){
        duty += inputs[DUTY_CV_INPUT].getVoltage() / 10.f;
    }
	_trainDuty = duty;
	_pulsar->duty(clamp(duty, 0.f, 1.f));

    float rampLen = params[RAMP_PCT_PARAM].getValue();
    if(inputs[ENV_RAMP_CV_INPUT].isConnected()){
        rampLen += inputs[ENV_RAMP_CV_INPUT].getVoltage() / 20.f;
    }
    _env->envRampLength(clamp(rampLen, 0.f, 0.5f));

    // check for plugging or unplugging of external waveform
	if(_useExternalWave != inputs[AUDIO_INPUT].isConnected()) {
        _useExternalWave = ! _useExternalWave;
        if(_useExternalWave) {
            _pulsar->setOsc(_extOsc);
        } else {
            _pulsar->setOsc(_osc);
        }
    }

    // only do this if we're NOT using the external waveform source
    if(! _useExternalWave) {
        // set the frequency
        float baseFreq = params[FREQ_PARAM].getValue();
        float voct = inputs[VOCT_INPUT].getVoltage();
        _freq = baseFreq * pow(2.f, voct);
        _osc->freq(_freq);

        // set the waveform
        float waveType = params[WAVE_TYPE_PARAM].getValue();
        if(_waveType != waveType) {
            _waveType = waveType;
            setWaveform();
        }
    }

    // set the envelope ramp type
	int rampType = params[RAMP_TYPE_PARAM].getValue();
	if(_rampType != rampType) {
		_rampType = rampType;
		setEnvelope();
	}

    _trainLength = params[TRAIN_LENGTH_PARAM].getValue();
    _trainAttack = params[TRAIN_RAMP_PARAM].getValue();
}


void PulseTrain2::processAudio(const ProcessArgs& args) {
    float audioOut = 0.f;

    if(inputs[TRIGGER_INPUT].isConnected()) {
        if(_trainTrig.process(inputs[TRIGGER_INPUT].getVoltage())) {
            // create new train
				PulsarTrainPtr pt;
			if(_useExternalWave) {
				pt = PulsarTrainPtr(new PulsarTrain(_extOsc, _env, _pulsarLength, _trainDuty, _trainLength, _trainAttack));
			} else {
				pt = PulsarTrainPtr(new PulsarTrain(_osc, _env, _pulsarLength, _trainDuty, _trainLength, _trainAttack));
				_osc = WTFOscPtr(new WTFOsc(_wavetable, 20, args.sampleRate));
			}
			_env = LUTEnvelopePtr(new LUTEnvelope(_lut, 100, 0.2f));
			_pulsars.push_back(pt);
        }

		size_t numInputs = 0;
		std::list<PulsarTrainPtr>::iterator it = _pulsars.begin();
		while(it != _pulsars.end()) {
			PulsarTrainPtr p = *it;
			if(p->isRunning()) {
				++it;
				audioOut += p->nextSample();
				numInputs++;
			} else {
				it = _pulsars.erase(it);
			}
		}

		audioOut = (numInputs == 0) ? 0.f : 5.f * audioOut / numInputs;
	}

    // run the next input sample through the thru-osc
    if(inputs[AUDIO_INPUT].isConnected()) {
        float input = inputs[AUDIO_INPUT].getVoltage() / 5.f;
        _extOsc->setNext(input);
    }
	
    // set the outputs
	outputs[AUDIO_OUTPUT].setVoltage(audioOut);
}




//------------------------------------------------------------
struct PulseTrain2Widget : ModuleWidget {

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




	PulseTrain2Widget(PulseTrain2* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PulseTrain.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 20;
		float rowY = 18;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, PulseTrain2::TRIGGER_INPUT));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, PulseTrain2::TRAIN_LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, PulseTrain2::TRAIN_RAMP_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, PulseTrain2::FREQ_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, PulseTrain2::WAVE_TYPE_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, PulseTrain2::RAMP_TYPE_PARAM));


		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, PulseTrain2::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, PulseTrain2::DUTY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, PulseTrain2::RAMP_PCT_PARAM));

		rowY += rowInc;

		// rowY = 69.f;
		rowY = 74.f;

		// top row of jacks
		rowY = 87.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, PulseTrain2::LENGTH_CV_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, PulseTrain2::DUTY_CV_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col3, rowY)), module, PulseTrain2::ENV_RAMP_CV_INPUT));

		// middle row of jacks
		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, PulseTrain2::VOCT_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, PulseTrain2::AUDIO_INPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, PulseTrain2::TRIGGER_OUTPUT));

		// bottom row of jacks
		rowY = 113.f;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col1, rowY)), module, PulseTrain2::WAVE_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col2, rowY)), module, PulseTrain2::ENV_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, PulseTrain2::AUDIO_OUTPUT));
	}
};

Model* modelPulseTrain2 = createModel<PulseTrain2, PulseTrain2Widget>("PulseTrain2");