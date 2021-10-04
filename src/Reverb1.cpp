#include "plugin.hpp"
#include "AllPassFilter.hpp"
#include "CombFilter.hpp"
#include "Reverb.hpp"
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
	int cf_delays[2][4] = {
		{1321, 1447, 1657, 1993},
		{1321, 1543, 1777, 1993}
	};
	int cf_delay_idx = 0;
	const size_t num_combs = 4;
	CombFilter* combs[4];

	/*
	 * I calculated these, again based on Schroeder's suggestion, 
	 * of 1-5msec. This is the list of primes to choose from:
	 * [43, 47, 53, 61, 71, 73, 79, 83, *89, 97, 101, 103, 107, 109, 113,
	 *  127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191,
	 *  193, 197, 199, 211, 223, 227, 229, *233]
	 */

	int apf_delays[4][2] = {
		{43,47},
		{43,233},
		{89,149},
		{89,233}
	};
	int apf_delay_idx = 3;
	const size_t num_apfs = 2;
	AllPassFilter* apfs[2];

	Reverb1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RT60_PARAM, 0.f, 10.f, 0.f, "RT60 (secs)");
		configParam(APG_PARAM, 0.f, 0.99f, 0.5f, "g for the all-pass stage");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Wet/Dry Mix");

		initFilters(cf_delays[cf_delay_idx], apf_delays[apf_delay_idx], 0.5f);
	}

	void initFilters(int* comb_freqs, int* apf_freqs, float g) {
		float rt60 = 1.f;
		int sample_rate = 44100;
		for(size_t i=0; i < num_combs; i++) {
			int freq = comb_freqs[i];
			combs[i] = new CombFilter(freq);
			combs[i]->g(calcCFG(freq, rt60, sample_rate));
		}

		for(size_t i=0; i < num_apfs; i++) {
			int freq = apf_freqs[i];
			apfs[i] = new AllPassFilter(freq);
			apfs[i]->g(g);
		}
	}

	const float wet_gain = 0.75f; //08f;
	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
		float rt60 = params[RT60_PARAM].getValue();
		float apg = params[APG_PARAM].getValue();

		cookParams(rt60, apg, args.sampleRate);

		float comb_step = combs[0]->process(input) + combs[1]->process(input) +
						combs[2]->process(input) + combs[3]->process(input);
		float wet_out = wet_gain * apfs[1]->process(apfs[0]->process(comb_step));

		float mix = params[MIX_PARAM].getValue();
		float out = ((1-mix) * input) + (mix * wet_out); // this should prob use log scale

		outputs[AUDIO_OUTPUT].setVoltage(out);
	}

	void cookParams(float rt60, float apg, int sample_rate) {

		combs[0]->g(calcCFG(cf_delays[cf_delay_idx][0], rt60, sample_rate));
		combs[1]->g(calcCFG(cf_delays[cf_delay_idx][1], rt60, sample_rate));
		combs[2]->g(calcCFG(cf_delays[cf_delay_idx][2], rt60, sample_rate));
		combs[3]->g(calcCFG(cf_delays[cf_delay_idx][3], rt60, sample_rate));

		apfs[0]->g(apg);
		apfs[1]->g(apg);
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