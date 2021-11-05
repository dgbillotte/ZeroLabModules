#include "plugin.hpp"

#include "lib/Components.hpp"
#include "lib/KarplusStrong.hpp"
#include "lib/SmithAngellResonator.hpp"

struct PlucktX3 : Module {
	enum ParamIds {
		STRING_DELAY_PARAM,
		PLUCK_FREQ_PARAM,
		DECAY_PARAM,
		STRETCH_PARAM,
		ATTACK_PARAM,
		PICK_POS_ON_PARAM,
		PICK_POS_PARAM,
		IMPULSE_LPF_ON_PARAM,
		IMPULSE_LPF_PARAM,
		BODY_SIZE_PARAM,
		RES_Q_PARAM,
		RES_MIX_PARAM,
		IMPULSE_TYPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PLUCK_INPUT,
		REFRET_INPUT,
		PLUCK_VOCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		DRY_OUTPUT,
		MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const int MAX_DELAY;
	const float E2 = 82.41f;
	KarplusStrong _eString;
	KarplusStrong _aString;
	KarplusStrong _dString;
	SmithAngellResonator _resonator;

	dsp::SchmittTrigger _pluckTrig;
	dsp::SchmittTrigger _refretTrig;

	PlucktX3() :
		MAX_DELAY(5000),
		_eString(APP->engine->getSampleRate(), MAX_DELAY),
		_aString(APP->engine->getSampleRate(), MAX_DELAY),
		_dString(APP->engine->getSampleRate(), MAX_DELAY),
		_resonator(APP->engine->getSampleRate())
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STRING_DELAY_PARAM, 30.f, 1000.f, 50, "Delay between strings on strum");
		// configParam(PLUCK_FREQ_PARAM, 30.f, 10000.f, E2, "Pluck Frequency");
		configParam(DECAY_PARAM, 0.0f, 1.414f, 1.f, "Decay");
		configParam(STRETCH_PARAM, 0.f, 1.f, 0.5f, "Stretch");
		configParam(ATTACK_PARAM, 0.f, 10.f, 1.f, "Attack");

		configParam(PICK_POS_ON_PARAM, 0.f, 1.0f, 0.f, "Pick Position On/Off");
		configParam(PICK_POS_PARAM, 0.f, 1.f, 0.1f, "Pick Position");
		configParam(IMPULSE_LPF_ON_PARAM, 0.f, 1.0f, 0.f, "Impulse LPF On/Off");
		configParam(IMPULSE_LPF_PARAM, 20.f, 10000.f, 5000.f, "Impulse LPF");

		configParam(BODY_SIZE_PARAM, 0.1f, 5.0f, 1.f, "Resonance Body Size");
		configParam(RES_Q_PARAM, 0.1f, 2.0f, 1.f, "Resonance Q");
		configParam(RES_MIX_PARAM, 0.f, 1.0f, 0.f, "Resonance Mix");

		configParam(IMPULSE_TYPE_PARAM, KarplusStrong::WHITE_NOISE+0.5f,
			KarplusStrong::NOISE_OTF+0.49f, KarplusStrong::WHITE_NOISE+0.5f, "Impulse Type");
	}

	void onSampleRateChange() override;
	float _gain = 5.f;
	int downsampleCount = 0;
	int downsampleRate = 16;
	float resMixSave = 0;

	int _stringDelay = 1000;
	int _delayAPluck = 0;
	int _delayDPluck = 0;
	float _rootFreqSave = 0.f;

	void process(const ProcessArgs& args) override;

	float sigmoidX2(float x);
	const float SPEED_OF_SOUND = 1125.f; // feet/sec
	float _lengthToFreq(float lengthFeet) {
		return SPEED_OF_SOUND / lengthFeet; 
	}	
};

void PlucktX3::onSampleRateChange() {
	int sampleRate = APP->engine->getSampleRate();
	_eString.sampleRate(sampleRate);
	_aString.sampleRate(sampleRate);
	_dString.sampleRate(sampleRate);
	_resonator.sampleRate(sampleRate);
}

// requires x in [0,1.414]
float PlucktX3::sigmoidX2(float x) {
	return (x <= 0.7071f) ? x * x : -(x - 1.414f) * (x - 1.414f) + 1.f;
}

void PlucktX3::process(const ProcessArgs& args) {
	// only process non-audio params every downsampleRate samples
	if(downsampleCount++ == downsampleRate) {
		downsampleCount = 0;

		float decay = sigmoidX2(params[DECAY_PARAM].getValue());
		decay = decay * 0.3f + 0.7f;
		float stretch = params[STRETCH_PARAM].getValue();
		float bodyLength = params[BODY_SIZE_PARAM].getValue();
		float resQ = params[RES_Q_PARAM].getValue();
		resMixSave = params[RES_MIX_PARAM].getValue();

		_eString.p(decay);
		_eString.S(stretch);
		_aString.p(decay);
		_aString.S(stretch);
		_dString.p(decay);
		_dString.S(stretch);
		_resonator.freq(_lengthToFreq(bodyLength));
		_resonator.q(resQ);


		if(params[PICK_POS_ON_PARAM].getValue() == 1) {
			_eString.pickPosOn(true);
			_aString.pickPosOn(true);
			_dString.pickPosOn(true);
			float pickPos = audioTaperX2(params[PICK_POS_PARAM].getValue());
			_eString.pickPos(pickPos);
			_aString.pickPos(pickPos);
			_dString.pickPos(pickPos);
		} else {
			_eString.pickPosOn(false);
			_aString.pickPosOn(false);
			_dString.pickPosOn(false);
		}

		if(params[IMPULSE_LPF_ON_PARAM].getValue() == 1) {
			_eString.impulseLpfOn(true);
			_aString.impulseLpfOn(true);
			_dString.impulseLpfOn(true);
			float lpfFreq = params[IMPULSE_LPF_PARAM].getValue();
			_eString.impulseLpfFreq(lpfFreq);
			_aString.impulseLpfFreq(lpfFreq);
			_dString.impulseLpfFreq(lpfFreq);
		} else {
			_eString.impulseLpfOn(false);
			_aString.impulseLpfOn(false);
			_dString.impulseLpfOn(false);
		}



		// if there is a trigger, initiate a new pluck
		float pluck = inputs[PLUCK_INPUT].getVoltage();
		bool pluckE = _pluckTrig.process(pluck);
		bool pluckA = (_delayAPluck != 0 && --_delayAPluck == 0);
		bool pluckD = (_delayDPluck != 0 && --_delayDPluck == 0);
		if(pluckE || pluckA || pluckD) {
			float stringDelay = params[STRING_DELAY_PARAM].getValue();
			float voct = inputs[PLUCK_VOCT_INPUT].getVoltage();
			_rootFreqSave = E2 * pow(2.f, voct);
			float attack = params[ATTACK_PARAM].getValue();
			float impulseType = params[IMPULSE_TYPE_PARAM].getValue();
			if(pluckE) {
				_eString.pluck(_rootFreqSave, attack, impulseType);
				_delayAPluck = stringDelay;
				_delayDPluck = 2 * stringDelay;
			}
			if(pluckA)
				_aString.pluck(_rootFreqSave * 1.4982f, attack, impulseType);
			if(pluckD)
				_dString.pluck(_rootFreqSave * 2.f, attack, impulseType);

		} else { // only do a refret if there wasn't a pluck
			float refret = inputs[REFRET_INPUT].getVoltage();
			if (_refretTrig.process(refret)) {
				float voct = inputs[PLUCK_VOCT_INPUT].getVoltage();
				float rootFreq = E2 * pow(2.f, voct);
				float fifthFreq = rootFreq * 1.4982f;
				_eString.refret(rootFreq);
				_aString.refret(fifthFreq);
				_dString.refret(rootFreq*2.f);
			}
		}	
	}

	float dryOut = (_eString.nextValue() + _aString.nextValue() + _dString.nextValue())/3.f;
	float wetOut = _resonator.process(dryOut);
	float mixOut = (resMixSave * wetOut) + ((1-resMixSave) * dryOut);

	outputs[DRY_OUTPUT].setVoltage(dryOut * _gain);
	outputs[MIX_OUTPUT].setVoltage(mixOut * _gain);

}

struct PlucktX3Widget : ModuleWidget {

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

	PlucktX3Widget(PlucktX3* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PlucktX3.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, PlucktX3::STRING_DELAY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, PlucktX3::DECAY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, PlucktX3::STRETCH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, PlucktX3::ATTACK_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, PlucktX3::PICK_POS_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, PlucktX3::IMPULSE_LPF_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col1, rowY)), module, PlucktX3::BODY_SIZE_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col2, rowY)), module, PlucktX3::RES_Q_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col3, rowY)), module, PlucktX3::RES_MIX_PARAM));

		rowY += rowInc+10;
		addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(Vec(col2, rowY)), module, PlucktX3::IMPULSE_TYPE_PARAM));


		// top row of jacks
		rowY = 87.f;

		// middle row of jacks
		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, PlucktX3::PLUCK_VOCT_INPUT));
		addParam(createParamCentered<NKK>(mm2px(Vec(col2, rowY)), module, PlucktX3::PICK_POS_ON_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(col3, rowY)), module, PlucktX3::IMPULSE_LPF_ON_PARAM));
		// bottom row of jacks
		rowY = 113.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, PlucktX3::PLUCK_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(3*_8th, rowY)), module, PlucktX3::REFRET_INPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(5*_8th, rowY)), module, PlucktX3::DRY_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(_7_8th, rowY)), module, PlucktX3::MIX_OUTPUT));
	}
};


Model* modelPlucktX3 = createModel<PlucktX3, PlucktX3Widget>("PlucktX3");