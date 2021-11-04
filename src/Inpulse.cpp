#include "plugin.hpp"

#include "lib/Components.hpp"
#include "lib/KarplusStrong.hpp"
#include "lib/SmithAngellResonator.hpp"


struct Inpulse : Module {
	enum ParamIds {
		PLUCK_FREQ_PARAM,
		DECAY_PARAM,
		STRETCH_PARAM,
		ATTACK_PARAM,
		PICK_POS_ON_PARAM,
		PICK_POS_PARAM,
		IMPULSE_LPF_ON_PARAM,
		IMPUSE_LPF_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PLUCK_INPUT,
		REFRET_INPUT,
		PLUCK_VOCT_INPUT,
		IMPULSE_SAMPLE_INPUT,
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
	const float IMPULSE_INPUT_GAIN = 0.2f;
	KarplusStrong _kpString;



	dsp::SchmittTrigger _pluckTrig;
	dsp::SchmittTrigger _refretTrig;

	// const float BASE_FREQ = 261.6256f;

	Inpulse() :
		MAX_DELAY(5000),
		_kpString(APP->engine->getSampleRate(), MAX_DELAY)
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PLUCK_FREQ_PARAM, 82.41f, 220.f, 82.41f, "Pluck Frequency");
		configParam(DECAY_PARAM, 0.0f, 1.414f, 1.f, "Decay");
		configParam(STRETCH_PARAM, 0.f, 1.f, 0.5f, "Stretch");

		configParam(ATTACK_PARAM, 0.f, 10.f, 1.f, "Attack");

		configParam(PICK_POS_ON_PARAM, 0.f, 1.0f, 0.f, "Pick Position On/Off");
		configParam(PICK_POS_PARAM, 0.f, 1.f, 0.1f, "Pick Position");
		configParam(IMPULSE_LPF_ON_PARAM, 0.f, 1.0f, 0.f, "Impulse LPF On/Off");
		configParam(IMPUSE_LPF_PARAM, 20.f, 5000.f, 5000.f, "Impulse LPF");

	}

	void onSampleRateChange() override;

	float _gain = 5.f;
	int downsampleCount = 0;
	int downsampleRate = 16;
	float resMixSave = 0;
	void process(const ProcessArgs& args) override;

	float sigmoidX2(float x);
	const float SPEED_OF_SOUND = 1125.f; // feet/sec
	float _lengthToFreq(float lengthFeet) {
		return SPEED_OF_SOUND / lengthFeet; 
	}
};

void Inpulse::onSampleRateChange() {
	int sampleRate = APP->engine->getSampleRate();
	_kpString.sampleRate(sampleRate);
	// _resonator.sampleRate(sampleRate);
}

// requires x in [0,1.414]
float Inpulse::sigmoidX2(float x) {
	return (x <= 0.7071f) ? x * x : -(x - 1.414f) * (x - 1.414f) + 1.f;
}

void Inpulse::process(const ProcessArgs& args) {
	// only process non-audio params every downsampleRate samples
	if(downsampleCount++ == downsampleRate) {
		downsampleCount = 0;
		float decay = sigmoidX2(params[DECAY_PARAM].getValue());
		decay = decay * 0.3f + 0.7f;
		float stretch = params[STRETCH_PARAM].getValue();

		_kpString.p(decay);
		_kpString.S(stretch);

		if(params[PICK_POS_ON_PARAM].getValue() == 1) {
			_kpString.pickPosOn(true);
			float pickPos = audioTaperX2(params[PICK_POS_PARAM].getValue());
			_kpString.pickPos(pickPos);
		} else {
			_kpString.pickPosOn(false);
		}

		if(params[IMPULSE_LPF_ON_PARAM].getValue() == 1) {
			_kpString.dynamicsOn(true);
			float lpfFreq = params[IMPUSE_LPF_PARAM].getValue();
			_kpString.lpfFreq(lpfFreq);
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
		float impulseType = 0;//params[IMPULSE_TYPE_PARAM].getValue();
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

	// capture the impulse input and hand it to the string model 
	// for use in on-the-fly impulse generation
	float impulseSample = inputs[IMPULSE_SAMPLE_INPUT].getVoltage() * IMPULSE_INPUT_GAIN;

	_kpString.setOTFSample(impulseSample);

	float dryOut = _kpString.nextValue();
	outputs[DRY_OUTPUT].setVoltage(dryOut * _gain);

}


//------------------------------------------------------------
struct InpulseWidget : ModuleWidget {

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



	InpulseWidget(Inpulse* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Inpulse.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Inpulse::PLUCK_FREQ_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Inpulse::DECAY_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Inpulse::STRETCH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Inpulse::ATTACK_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Inpulse::PICK_POS_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Inpulse::IMPUSE_LPF_PARAM));

		rowY += rowInc;

		// rowY += rowInc+10;
		addParam(createParamCentered<NKK>(mm2px(Vec(col2, rowY)), module, Inpulse::PICK_POS_ON_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(col3, rowY)), module, Inpulse::IMPULSE_LPF_ON_PARAM));

		// top row of jacks
		rowY = 87.f;

		// middle row of jacks
		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, Inpulse::PLUCK_VOCT_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, Inpulse::IMPULSE_SAMPLE_INPUT));

		// bottom row of jacks
		rowY = 113.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, Inpulse::PLUCK_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, Inpulse::REFRET_INPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, Inpulse::DRY_OUTPUT));
	}
};

Model* modelInpulse = createModel<Inpulse, InpulseWidget>("Inpulse");