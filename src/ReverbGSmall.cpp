#include "plugin.hpp"
#include "lib/GardnerReverbs.hpp"
#include "lib/ReverbS.hpp"
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

	GardnerReverbSmall reverb1 = GardnerReverbSmall(APP->engine->getSampleRate());
	// GardnerReverbMed reverb = GardnerReverbMed(APP->engine->getSampleRate());
	// GardnerReverbLarge reverb = GardnerReverbLarge(APP->engine->getSampleRate());
	ReverbS reverb2 = ReverbS(APP->engine->getSampleRate());


	ReverbGSmall() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PREDELAY_PARAM, 0.f, 10.f, 0.f, "Pre-Delay");
		configParam(OTHER_PARAM, 0.f, 1.f, 0.f, "Long Delay");
		configParam(LPF_FREQ_PARAM, 100.f, 3000.f, 200.f, "LPF Frequency");
		configParam(G_PARAM, 0.f, 0.865f, 0.5f, "G");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Dry/Wet Mix");
	}


	// handle changes to the sample rate
	void onSampleRateChange() override {
		reverb1.sampleRate(APP->engine->getSampleRate());
		reverb2.sampleRate(APP->engine->getSampleRate());
	}

    int count = 0;
	void process(const ProcessArgs& args) override {
		// audio input
		float input = inputs[AUDIO_INPUT].getVoltage();
		
		// params + CVs
		// NOTE: might need to clamp some of these...
		float predelay = params[PREDELAY_PARAM].getValue() + inputs[PREDELAY_CV_INPUT].getVoltage();
		float other = params[OTHER_PARAM].getValue() + inputs[OTHER_CV_INPUT].getVoltage();
		float lpfFreq = params[LPF_FREQ_PARAM].getValue() +	inputs[LPF_FREQ_CV_INPUT].getVoltage();
		float g = params[G_PARAM].getValue() + inputs[G_CV_INPUT].getVoltage();
		float mix = params[MIX_PARAM].getValue() + inputs[MIX_CV_INPUT].getVoltage();

		// reverb.preDelay(predelay);
		// reverb.longDelay(other);
		reverb1.g(g);
		reverb1.lpfFreq(lpfFreq);
		reverb2.rt60(predelay);
		reverb2.apfG(other);

		float output1 = reverb1.process(input);
		float output2 = reverb2.process(input);
		// float output = output2;
		float output = (output1+output2)/2;
		// float output = reverb1.process(reverb2.process(input));

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
	float width = 50.8;
	float midX = width/2;
	float height = 128.5;
	float _8th = width/8;
	float _7_8th = width-_8th;
	float gutter = 5.f;

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
		addParam(createParamCentered<Davies1900hLargeWhiteKnob>(mm2px(Vec(_8th+gutter, rowY)), module, ReverbGSmall::PREDELAY_PARAM));
		addParam(createParamCentered<Davies1900hLargeWhiteKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, ReverbGSmall::OTHER_PARAM));

		// LPF, Mix, & G
		rowY = 50.f;
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(_8th+gutter, rowY)), module, ReverbGSmall::LPF_FREQ_PARAM));
		addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(Vec(midX, rowY + 20)), module, ReverbGSmall::MIX_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, ReverbGSmall::G_PARAM));

		// jacks
		rowY = 98.f;
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_8th, rowY)), module, ReverbGSmall::PREDELAY_CV_INPUT));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(3*_8th, rowY)), module, ReverbGSmall::OTHER_CV_INPUT));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5*_8th, rowY)), module, ReverbGSmall::LPF_FREQ_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_7_8th, rowY)), module, ReverbGSmall::G_CV_INPUT));

		rowY += 15.f;
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_8th, rowY)), module, ReverbGSmall::AUDIO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, ReverbGSmall::MIX_CV_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(_7_8th, rowY)), module, ReverbGSmall::MIX_OUTPUT));
	}
};


Model* modelReverbGSmall = createModel<ReverbGSmall, ReverbGSmallWidget>("ReverbGSmall");