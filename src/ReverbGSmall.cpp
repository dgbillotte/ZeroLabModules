#include "plugin.hpp"
#include "lib/GardnerReverbs.hpp"
/*
 * 
 */
struct ReverbGSmall : Module {
	enum ParamIds {
		DELAY_PARAM,
		G_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
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
		configParam(DELAY_PARAM, 100.f, 3000.f, 6000.f, "g delay");
		configParam(G_PARAM, 0.f, 1.f, 0.5f, "g");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Dry/Wet Mix");
	}

    int count = 0;

	void onSampleRateChange() override {
		reverb.sampleRate(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
		float delay = params[DELAY_PARAM].getValue();
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
	ReverbGSmallWidget(ReverbGSmall* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/1I1O2K.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float xMid = 10.5f;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 20.f)), module, Reverb1::WET_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 10.f)), module, ReverbGSmall::DELAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 25.f)), module, ReverbGSmall::G_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 40.f)), module, ReverbGSmall::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xMid, 100.f)), module, ReverbGSmall::AUDIO_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xMid, 115.f)), module, ReverbGSmall::MIX_OUTPUT));
	}
};


Model* modelReverbGSmall = createModel<ReverbGSmall, ReverbGSmallWidget>("ReverbGSmall");