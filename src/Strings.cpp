#include "plugin.hpp"
#include "lib/DelayBuffer.hpp"
#include "lib/Components.hpp"
#include "lib/KarplusStrong.hpp"



struct Strings : Module {
	enum ParamIds {
		PLUCK_FREQ_PARAM,
		DECAY_PARAM,
		STRETCH_PARAM,
		ATTACK_PARAM,
		BODY_SIZE_PARAM,
		RES_Q_PARAM,
		RES_MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PLUCK_INPUT,
		PLUCK_VOCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const int MAX_DELAY;
	KarplusStrong _kpString;
	dsp::SchmittTrigger _trigger;

	const float BASE_FREQ = 261.6256f;

	Strings() :	MAX_DELAY(5000), _kpString(APP->engine->getSampleRate(), MAX_DELAY) {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PLUCK_FREQ_PARAM, 40.f, 1000.f, BASE_FREQ, "Pluck Frequency");
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay");
		configParam(ATTACK_PARAM, 0.f, 10.f, 1.f, "Attack");
		configParam(STRETCH_PARAM, 0.f, 1.f, 0.5f, "Stretch");
		configParam(BODY_SIZE_PARAM, 0.1f, 10.0f, 1.f, "Resonance Body Size");
		configParam(RES_Q_PARAM, 0.1f, 10.0f, 1.f, "Resonance Q");
		configParam(RES_MIX_PARAM, 0.f, 1.0f, 0.f, "Resonance Mix");
	}

	void onSampleRateChange() override;
	int count = 0;
	void process(const ProcessArgs& args) override;
	void _excite();
};

void Strings::onSampleRateChange() {
	_kpString.sampleRate(APP->engine->getSampleRate());
}

void Strings::process(const ProcessArgs& args) {
	count++;

	float decay = params[DECAY_PARAM].getValue();
	float stretch = params[STRETCH_PARAM].getValue();
	// float bodySize = params[BODY_SIZE_PARAM].getValue();
	// float resMix = params[RES_MIX_PARAM].getValue();

	_kpString.p(decay);
	_kpString.S(stretch);
	// _kpString.bodySize(bodySize);
	// _kpString.resonanceMix(resMix);

	// if there is a trigger, initiate a new pluck
	float pluck = inputs[PLUCK_INPUT].getVoltage();
	if (_trigger.process(pluck)) {
		float baseFreq = params[PLUCK_FREQ_PARAM].getValue();
		float voct = inputs[PLUCK_VOCT_INPUT].getVoltage();
		float freq = baseFreq * pow(2.f, voct);
		float attack = params[ATTACK_PARAM].getValue();
		_kpString.pluck(freq, attack);
		count = 0;
	}

	// float testSW = params[TEST_SW_PARAM].getValue(); // 0,1,2

	// output the next sample
	float out = _kpString.nextValue(count < 20);
	outputs[AUDIO_OUTPUT].setVoltage(out);
}

// These are for converting from length to frequency/delay-time
// const float SPEED_OF_SOUND = 1125.f; // feet/sec
// int _sizeToDelay(float lengthFeet) {
//     float secs = lengthFeet/SPEED_OF_SOUND;
//     return secs * _sampleRate;
// }

// float _sizeToFreq(float lengthFeet) {
//     return SPEED_OF_SOUND / lengthFeet; 
// }


//------------------------------------------------------------
struct StringsWidget : ModuleWidget {

	float width = 20.32f;
	float midX = width/2.f;
	StringsWidget(Strings* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Strings.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 15;
		float rowY = 18;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::PLUCK_FREQ_PARAM));

		// rowY += rowInc;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::DECAY_PARAM));

		// rowY += rowInc;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::STRETCH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::ATTACK_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::BODY_SIZE_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::RES_Q_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::RES_MIX_PARAM));


		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(midX, rowY)), module, Strings::PLUCK_INPUT));

		rowY += 10;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(midX, rowY)), module, Strings::PLUCK_VOCT_INPUT));

		rowY += 10;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(midX, rowY)), module, Strings::AUDIO_OUTPUT));
	}
};

Model* modelStrings = createModel<Strings, StringsWidget>("Strings");