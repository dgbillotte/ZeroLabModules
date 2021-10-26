#include "plugin.hpp"
// #include "lib/DelayBuffer.hpp"
#include "lib/Components.hpp"
#include "lib/KarplusStrong.hpp"
#include "lib/SmithAngellResonator.hpp"


struct Strings : Module {
	enum ParamIds {
		PLUCK_FREQ_PARAM,
		DECAY_PARAM,
		STRETCH_PARAM,
		ATTACK_PARAM,
		PICK_POS_ON_PARAM,
		PICK_POS_PARAM,
		IMPULSE_LPF_ON_PARAM,
		IMPUSE_LPF_PARAM,
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
	KarplusStrong _kpString;
	SmithAngellResonator _resonator = SmithAngellResonator(APP->engine->getSampleRate());

	dsp::SchmittTrigger _pluckTrig;
	dsp::SchmittTrigger _refretTrig;

	// const float BASE_FREQ = 261.6256f;

	Strings() :	MAX_DELAY(5000), _kpString(APP->engine->getSampleRate(), MAX_DELAY) {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PLUCK_FREQ_PARAM, 82.41f, 220.f, 82.41f, "Pluck Frequency");
		// configParam(DECAY_PARAM, 0.7f, 1.f, 1.f, "Decay");
		configParam(DECAY_PARAM, 0.0f, 1.414f, 1.f, "Decay");
		configParam(STRETCH_PARAM, 0.f, 1.f, 0.5f, "Stretch");

		configParam(ATTACK_PARAM, 0.f, 10.f, 1.f, "Attack");

		configParam(PICK_POS_ON_PARAM, 0.f, 1.0f, 0.f, "Pick Position On/Off");
		configParam(PICK_POS_PARAM, 0.f, 1.f, 0.1f, "Pick Position");
		configParam(IMPULSE_LPF_ON_PARAM, 0.f, 1.0f, 0.f, "Impulse LPF On/Off");
		configParam(IMPUSE_LPF_PARAM, 20.f, 5000.f, 5000.f, "Impulse LPF");

		configParam(BODY_SIZE_PARAM, 0.1f, 5.0f, 1.f, "Resonance Body Size");
		configParam(RES_Q_PARAM, 0.1f, 2.0f, 1.f, "Resonance Q");
		configParam(RES_MIX_PARAM, 0.f, 1.0f, 0.f, "Resonance Mix");

		configParam(IMPULSE_TYPE_PARAM, KarplusStrong::WHITE_NOISE+0.5f,
			KarplusStrong::NOISE_OTF+0.49f, KarplusStrong::WHITE_NOISE+0.5f, "Impulse Type");
	}

	void onSampleRateChange() override;
	void process(const ProcessArgs& args) override;

	const float SPEED_OF_SOUND = 1125.f; // feet/sec
	float _lengthToFreq(float lengthFeet) {
		return SPEED_OF_SOUND / lengthFeet; 
	}
};

void Strings::onSampleRateChange() {
	int sampleRate = APP->engine->getSampleRate();
	_kpString.sampleRate(sampleRate);
	_resonator.sampleRate(sampleRate);
}


float _gain = 5.f;
int downsampleCount = 0;
int downsampleRate = 16;
float resMixSave = 0;
void Strings::process(const ProcessArgs& args) {
	// only process non-audio params every downsampleRate samples
	if(downsampleCount++ ==  downsampleRate) {
		downsampleCount = 0;
		float decay = params[DECAY_PARAM].getValue();
		if(decay <= 0.7071f) {
			decay = decay * decay;
		} else {
			decay = -(decay-1.414f)*(decay-1.414f) + 1.f;
		}
		float stretch = params[STRETCH_PARAM].getValue();
		float bodyLength = params[BODY_SIZE_PARAM].getValue();
		float resQ = params[RES_Q_PARAM].getValue();
		resMixSave = params[RES_MIX_PARAM].getValue();

		_kpString.p(decay);
		_kpString.S(stretch);
		_resonator.freq(_lengthToFreq(bodyLength));
		_resonator.q(resQ);


		if(params[PICK_POS_ON_PARAM].getValue() == 1) {
			_kpString.pickPosOn(true);
			float pickPos = audioTaperX2(params[PICK_POS_PARAM].getValue());
			_kpString.pickPos(pickPos);
		} else {
			_kpString.pickPosOn(false);
		}

		if(params[IMPULSE_LPF_ON_PARAM].getValue() == 1) {
			_kpString.dynamicsOn(true);
			float dynamicLevel = params[IMPUSE_LPF_PARAM].getValue();
			// _kpString.dynamicsLevel(1/dynamicLevel);
			_kpString.lpfFreq(dynamicLevel);
		} else {
			_kpString.dynamicsOn(false);
		}
	}

	// if there is a trigger, initiate a new pluck
	float pluck = inputs[PLUCK_INPUT].getVoltage();

	if (_pluckTrig.process(pluck)) {
		float baseFreq = params[PLUCK_FREQ_PARAM].getValue();
		float voct = inputs[PLUCK_VOCT_INPUT].getVoltage();
		float freq = baseFreq * pow(2.f, voct);
		float attack = params[ATTACK_PARAM].getValue();
		float impulseType = params[IMPULSE_TYPE_PARAM].getValue();
		_kpString.pluck(freq, attack, impulseType);

	} else { // only do a refret if there wasn't a pluck
		float refret = inputs[REFRET_INPUT].getVoltage();
		if (_refretTrig.process(refret)) {
			float baseFreq = params[PLUCK_FREQ_PARAM].getValue();
			float voct = inputs[PLUCK_VOCT_INPUT].getVoltage();
			float freq = baseFreq * pow(2.f, voct);
			_kpString.refret(freq);
		}
	}

	// generate the dry output
	float dryOut = _kpString.nextValue();

	// pipe through the resonator
	float wetOut = _resonator.process(dryOut);

	float mixOut = (resMixSave * wetOut) + ((1-resMixSave) * dryOut);

	outputs[DRY_OUTPUT].setVoltage(dryOut * _gain);
	outputs[MIX_OUTPUT].setVoltage(mixOut * _gain);
}

// These are for converting from length to frequency/delay-time
// int _sizeToDelay(float lengthFeet) {
//     float secs = lengthFeet/SPEED_OF_SOUND;
//     return secs * _sampleRate;
// }



//------------------------------------------------------------
struct StringsWidget : ModuleWidget {

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


	StringsWidget(Strings* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Strings.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Strings::PLUCK_FREQ_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Strings::DECAY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Strings::STRETCH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Strings::ATTACK_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Strings::PICK_POS_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Strings::IMPUSE_LPF_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col1, rowY)), module, Strings::BODY_SIZE_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col2, rowY)), module, Strings::RES_Q_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col3, rowY)), module, Strings::RES_MIX_PARAM));

		rowY += rowInc+10;
		addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(Vec(col2, rowY)), module, Strings::IMPULSE_TYPE_PARAM));
		

		// top row of jacks
		rowY = 87.f;

		// middle row of jacks
		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, Strings::PLUCK_VOCT_INPUT));
		addParam(createParamCentered<NKK>(mm2px(Vec(col2, rowY)), module, Strings::PICK_POS_ON_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(col3, rowY)), module, Strings::IMPULSE_LPF_ON_PARAM));

		// bottom row of jacks
		rowY = 113.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, Strings::PLUCK_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(3*_8th, rowY)), module, Strings::REFRET_INPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(5*_8th, rowY)), module, Strings::DRY_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(_7_8th, rowY)), module, Strings::MIX_OUTPUT));
	}
};

Model* modelStrings = createModel<Strings, StringsWidget>("Strings");