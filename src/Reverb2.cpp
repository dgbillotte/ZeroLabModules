#include "plugin.hpp"
#include "AllPassFilter.hpp"
#include "DelayBuffer.hpp"
// #include "CombFilter.hpp"

/*
 * This plugin is based off Will Pirkle's book ...
 */


struct Reverb2 : Module {
	enum ParamIds {
		DELAY_PARAM,
		// APG_PARAM,
		// MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
        DRY_OUTPUT,
		WET_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    DelayBuffer<float> _delayLine;
    NestedAllPassFilter _apf1;

	Reverb2() : _delayLine(10), _apf1(&_delayLine, 1, 1, 0.5f) {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(DELAY_PARAM, 1, 100, 0, "Delay (samples)");
		// configParam(APG_PARAM, 0.f, 0.99f, 0.5f, "g for the all-pass stage");
		// configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Wet/Dry Mix");
	}

    int count = 0;

	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
        outputs[DRY_OUTPUT].setVoltage(input);

		float delay = params[DELAY_PARAM].getValue();
		// float apg = params[APG_PARAM].getValue();
		// float mix = params[MIX_PARAM].getValue();

		// reverb.rt60(rt60);
		// reverb.apfG(apg);
		// reverb.dryWetMix(mix);

        _apf1.delay(delay);
        float output = _apf1.process(input);

        // dump out the state of things
        if(count++ % 20000 == 0) {
            std::cout << "input: " << input << ", output: " << output << std::endl;
            _apf1.dump();
        }

		outputs[WET_OUTPUT].setVoltage(output);
	}
};


struct Reverb2Widget : ModuleWidget {
	Reverb2Widget(Reverb2* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/1I1O2K.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float xMid = 10.5f;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 20.f)), module, Reverb1::WET_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 20.f)), module, Reverb2::DELAY_PARAM));
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 35.f)), module, Reverb2::APG_PARAM));
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 50.f)), module, Reverb2::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xMid, 70.f)), module, Reverb2::AUDIO_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xMid, 85.f)), module, Reverb2::DRY_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xMid, 100.f)), module, Reverb2::WET_OUTPUT));
	}
};


Model* modelReverb2 = createModel<Reverb2, Reverb2Widget>("Reverb2");