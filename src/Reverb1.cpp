#include "plugin.hpp"
// #include "AllPassFilter.hpp"
// #include "CombFilter.hpp"
#include "lib/ReverbS.hpp"
/*
 * This plugin is based off Will Pirkle's book ...
 */

// this *need* to be zero-terminated :-D
int _combFreqsEven[] = {1321, 1543, 1777, 1993, 0};
int _combFreqsGR[] = {1321, 1447, 1657, 1993, 0};
int _apfFreqs[] = {89, 163, 0};
struct Reverb1 : Module {
	enum ParamIds {
		RT60_PARAM,
		APG_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	ReverbS reverb = ReverbS(); 

	Reverb1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RT60_PARAM, 0.f, 10.f, 0.f, "RT60 (secs)");
		configParam(APG_PARAM, 0.f, 0.99f, 0.5f, "g for the all-pass stage");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Wet/Dry Mix");
	}

	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
		float rt60 = params[RT60_PARAM].getValue();
		float apg = params[APG_PARAM].getValue();
		float mix = params[MIX_PARAM].getValue();

		reverb.rt60(rt60);
		reverb.apfG(apg);
		reverb.dryWetMix(mix);

		outputs[AUDIO_OUTPUT].setVoltage(reverb.process(input));
	}
};


struct Reverb1Widget : ModuleWidget {
	Reverb1Widget(Reverb1* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/1I1O2K.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float xMid = 10.5f;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 20.f)), module, Reverb1::WET_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 20.f)), module, Reverb1::RT60_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 35.f)), module, Reverb1::APG_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 50.f)), module, Reverb1::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xMid, 85.f)), module, Reverb1::AUDIO_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xMid, 100.f)), module, Reverb1::AUDIO_OUTPUT));
	}
};


Model* modelReverb1 = createModel<Reverb1, Reverb1Widget>("Reverb1");