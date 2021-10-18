#include "plugin.hpp"
#include "lib/SmithAngellResonator.hpp"

struct ResBody : Module {
	enum ParamIds {
		FREQKNOB_PARAM,
		Q_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	SmithAngellResonator _resonator = SmithAngellResonator(APP->engine->getSampleRate());

	ResBody() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 10.f, 10000.f, 0.f, "Frequency");
		configParam(Q_PARAM, 0.f, 11.f, 0.f, "Q");
		configParam(MIX_PARAM, 0.f, 1.f, 0.5f, "Dry/Wet Mix");
	}

    void onSampleRateChange() override {
        _resonator.sampleRate(APP->engine->getSampleRate());
    }

	void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        float q = params[Q_PARAM].getValue();
        
        _resonator.freq(freq);
        _resonator.q(q);

        float input = inputs[INPUT_INPUT].getVoltage();
        float output = _resonator.process(input);

		float mix = params[MIX_PARAM].getValue();
		output = (mix*output) + ((1-mix)*input);

        outputs[OUTPUT_OUTPUT].setVoltage(output);		
	}
};



struct ResBodyWidget : ModuleWidget {

	float width = 20.32f;
	float midX = width/2.f;

	ResBodyWidget(ResBody* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ResBody.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 15.f;
		float rowY = 18.f;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, ResBody::FREQKNOB_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, ResBody::Q_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, ResBody::MIX_PARAM));

		rowY = 90.f;
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, ResBody::INPUT_INPUT));

		rowY += rowInc;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, ResBody::OUTPUT_OUTPUT));
	}
};


Model* modelResBody = createModel<ResBody, ResBodyWidget>("ResBody");