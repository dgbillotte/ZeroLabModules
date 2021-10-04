#include "plugin.hpp"
#include "CombFilter.hpp"
#include "AllPassFilter.hpp"
/*
 * This plugin is based off Will Pirkle's book ...
 */


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
	
	/*
	 * I calculated the delays based on Schroeder's suggestion
	 * of 1:1.5 ratio for the delays of the comb filters, specifically
	 * 30-45msec. I calc'd this based on 44100 and then split the range
	 * up two ways. First was evenly, but I rounded to the nearest prime:
	 * D0: 1321, 1543, 1777, 1993
	 * The second is based on the golden ratio, again rounding to nearest prime
	 * D1: 1321, 1447, 1657, 1993
	 */

	// int cf_delays[] = {1321, 1447, 1657, 1993};
	// int cf_delays[] = {1321, 1543, 1777, 1993};
	CombFilter<1321> cf1;
	CombFilter<1543> cf2;
	CombFilter<1777> cf3;
	CombFilter<1993> cf4;

	/*
	 * I calculated these, again based on Schroeder's suggestion, 
	 * of 1-5msec. This is the list of primes to choose from:
	 * [43, 47, 53, 61, 71, 73, 79, 83, *89, 97, 101, 103, 107, 109, 113,
	 *  127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191,
	 *  193, 197, 199, 211, 223, 227, 229, *233]
	 */

	AllPassFilter<43> apf1;
	AllPassFilter<233> apf2;

	Reverb1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RT60_PARAM, 0.f, 10.f, 0.f, "RT60 (secs)");
		configParam(APG_PARAM, 0.5f, 0.707f, 0.5f, "g for the all-pass stage");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Wet/Dry Mix");
		// configParam(THRESH_CV_PARAM, 0.f, 1.f, 0.f, "");

		float rt60 = 0.f;
		int sample_rate = 44100;
		cf1.g(calcCFG(1321, rt60, sample_rate));
		cf2.g(calcCFG(1543, rt60, sample_rate));
		cf3.g(calcCFG(1777, rt60, sample_rate));
		cf4.g(calcCFG(1993, rt60, sample_rate));

		float apf_g = 0.5f;
		apf1.g(apf_g);
		apf2.g(apf_g);
	}

	const float wet_gain = 0.75f; //08f;
	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
		float rt60 = params[RT60_PARAM].getValue();
		float apg = params[APG_PARAM].getValue();

		cookParams(rt60, apg, args.sampleRate);

		float comb_step = cf1.process(input) + cf2.process(input) +
						cf3.process(input) + cf4.process(input);
		float wet_out = wet_gain * apf2.process(apf1.process(comb_step));


		float mix = params[MIX_PARAM].getValue();
		float out = ((1-mix) * input) + (mix * wet_out); // this should prob use log scale

		outputs[AUDIO_OUTPUT].setVoltage(out);
	}

	void cookParams(float rt60, float apg, int sample_rate) {

		cf1.g(calcCFG(1321, rt60, sample_rate));
		cf2.g(calcCFG(1543, rt60, sample_rate));
		cf3.g(calcCFG(1777, rt60, sample_rate));
		cf4.g(calcCFG(1993, rt60, sample_rate));

		apf1.g(apg);
		apf2.g(apg);
	}

	float calcCFG(size_t delay, float rt60, int sample_rate) {
		return pow(10.f, (-3.f*delay/sample_rate)/rt60);
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