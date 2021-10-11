#include "plugin.hpp"
#include "lib/GardnerReverbs.hpp"
/*
 * 
 */
struct ReverbGSmall : Module {
	enum ParamIds {
		PREDELAY_PARAM,
		OTHER_PARAM,
		LPF_FREQ_PARAM,
		G_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		PREDELAY_CV_INPUT,
		OTHER_CV_INPUT,
		LPF_FREQ_CV_INPUT,
		G_CV_INPUT,
		MIX_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// GardnerReverbSmall reverb = GardnerReverbSmall(APP->engine->getSampleRate());
	GardnerReverbMed reverb = GardnerReverbMed(APP->engine->getSampleRate());
	// GardnerReverbLarge reverb = GardnerReverbLarge(APP->engine->getSampleRate());

	ReverbGSmall() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LPF_FREQ_PARAM, 100.f, 3000.f, 6000.f, "g delay");
		configParam(G_PARAM, 0.f, 1.f, 0.5f, "g");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Dry/Wet Mix");
	}

    int count = 0;

	void onSampleRateChange() override {
		reverb.sampleRate(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
		float delay = params[LPF_FREQ_PARAM].getValue();
		float g = params[G_PARAM].getValue();
		float mix = params[MIX_PARAM].getValue();

		reverb.g(g);
		reverb.lpfFreq(delay);
		float output = reverb.process(input);

        // dump out the state of things
        int log = false;
        if(log && count++ % 20000 == 0) {
            std::cout << "input: " << input << ", output: " << output << std::endl;
            // _apf1.dump();
        }

        // mix the output
        output = mix*output + (1-mix)*input;
		outputs[MIX_OUTPUT].setVoltage(output);
	}
};


struct ReverbGSmallWidget : ModuleWidget {
	// module dimensions and handy guides
	float width = 50.80;
	float midX = width/2;
	float height = 128.5;
	float _8th = width/8;
	float _7_8th = width-_8th;

	ReverbGSmallWidget(ReverbGSmall* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Reverb.svg")));


		// screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowY = 25.f;
		// Delay, Other, and Reverb Type
		float gutter = 5.f;
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(_8th+gutter, rowY)), module, ReverbGSmall::PREDELAY_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, ReverbGSmall::OTHER_PARAM));

		// LPF, Mix, & G
		rowY = 50.f;
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(_8th+gutter, rowY)), module, ReverbGSmall::LPF_FREQ_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(midX, rowY + 20)), module, ReverbGSmall::MIX_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, ReverbGSmall::G_PARAM));

		// jacks
		rowY = 98.f;
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_8th, rowY)), module, ReverbGSmall::PREDELAY_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(3*_8th, rowY)), module, ReverbGSmall::OTHER_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5*_8th, rowY)), module, ReverbGSmall::LPF_FREQ_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_7_8th, rowY)), module, ReverbGSmall::G_CV_INPUT));

		rowY += 15.f;
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_8th, rowY)), module, ReverbGSmall::AUDIO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, ReverbGSmall::MIX_CV_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(_7_8th, rowY)), module, ReverbGSmall::MIX_OUTPUT));
	}
};


Model* modelReverbGSmall = createModel<ReverbGSmall, ReverbGSmallWidget>("ReverbGSmall");