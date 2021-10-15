#include "plugin.hpp"
#include "lib/DelayBuffer.hpp"
#include "lib/Components.hpp"
#include "lib/KarplusStrong.hpp"



struct Strings : Module {
	enum ParamIds {
		FILTER_FREQ_PARAM,
		PLUCK_FREQ_PARAM,
		TEST_PARAM,
		RESONANCE_PARAM,
		TEST_SW_PARAM,
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

	Strings() :	MAX_DELAY(10000), _kpString(APP->engine->getSampleRate()*2, MAX_DELAY) {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PLUCK_FREQ_PARAM, 40.f, 1000.f, BASE_FREQ, "Pluck Frequency");
		configParam(FILTER_FREQ_PARAM, 100.f, 10000.f, 2000.f, "LPF Frequency");
		configParam(TEST_PARAM, 0.1f, 10.f, 1.f, "Body Size (feet)");
		configParam(RESONANCE_PARAM, 0.f, 0.0f, 0.f, "Body Resonance");
		configParam(TEST_SW_PARAM, 0.f, 2.0f, 0.f, "Test Switch");
	}

	void onSampleRateChange() override;
	void process(const ProcessArgs& args) override;
	void _excite();
};

void Strings::onSampleRateChange() {
	_kpString.sampleRate(APP->engine->getSampleRate()*2);
}

void Strings::process(const ProcessArgs& args) {
	// if there is a trigger, initiate a new pluck
	float pluck = inputs[PLUCK_INPUT].getVoltage();
	if (_trigger.process(pluck)) {
		float baseFreq = params[PLUCK_FREQ_PARAM].getValue();
		float voct = inputs[PLUCK_VOCT_INPUT].getVoltage();
		float freq = baseFreq * pow(2.f, voct);
		_kpString.pluck(freq, 0);
	}
	
	// set all of the KP params
	float filterFreq = params[FILTER_FREQ_PARAM].getValue();
	float testSW = params[TEST_SW_PARAM].getValue(); // 0,1,2
	// float resonance = params[RESONANCE_PARAM].getValue();
	_kpString.lpfFreq(filterFreq);


	// output the next sample
	// running two samples to double the sampling frequency
	float out = _kpString.nextValue();
	out = _kpString.nextValue();
	outputs[AUDIO_OUTPUT].setVoltage(out);
}


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

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::FILTER_FREQ_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::TEST_PARAM));

		rowY += rowInc;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Strings::RESONANCE_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<NKK>(mm2px(Vec(midX, rowY)), module, Strings::TEST_SW_PARAM));


		rowY = 100.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(midX, rowY)), module, Strings::PLUCK_INPUT));

		rowY += 10;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(midX, rowY)), module, Strings::PLUCK_VOCT_INPUT));

		rowY += 10;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(midX, rowY)), module, Strings::AUDIO_OUTPUT));
	}
};

Model* modelStrings = createModel<Strings, StringsWidget>("Strings");