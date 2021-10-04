
#include "plugin.hpp"
#include "AllPassFilter.hpp"
#include "CombFilter.hpp"

struct APFilter : Module {
	enum ParamIds {
		G_PARAM,
		D1_PARAM,
		D2_PARAM,
		D3_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO1_OUTPUT,
		AUDIO2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// size_t MAX_DELAY = 1000;
	AllPassFilter apf1 = AllPassFilter(100);
	AllPassFilter apf2 = AllPassFilter(100);
	AllPassFilter apf3 = AllPassFilter(100);

	CombFilter cf1 = CombFilter(100);
	CombFilter cf2 = CombFilter(100);
	CombFilter cf3 = CombFilter(100);

	APFilter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(G_PARAM, -1.f, 1.f, 0.f, "g coefficient");
		configParam(D1_PARAM, 1, 30, 1, "filter delay");
		configParam(D2_PARAM, 0, 60, 0, "filter delay");
		configParam(D3_PARAM, 0, 50, 0, "filter delay");
	}

	void process(const ProcessArgs& args) override {
		float input = inputs[AUDIO_INPUT].getVoltage();
		float g = params[G_PARAM].getValue();
		float d1 = params[D1_PARAM].getValue();
		float d2 = params[D2_PARAM].getValue();
		float d3 = params[D3_PARAM].getValue();

		apf1.g(g);
		apf1.delay(d1);
		apf2.g(g);
		apf2.delay(d1+d2);
		apf3.g(g);
		apf3.delay(d1+d2+d3);

		// float out = (apf1.process(input) + apf2.process(input) + apf3.process(input))/3.f;
		float out = apf1.process(input);
		outputs[AUDIO1_OUTPUT].setVoltage(out);

		cf1.g(g);
		cf1.delay(d1);
		cf2.g(g);
		cf2.delay(d1+d2);
		cf3.g(g);
		cf3.delay(d1+d2+d3);

		out = (cf1.process(input) + cf2.process(input) + cf3.process(input))/3.f;
		outputs[AUDIO2_OUTPUT].setVoltage(out);

	}
};


struct APFilterWidget : ModuleWidget {
	APFilterWidget(APFilter* module) {
		float xMid = 10.5f;


		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/1I1O2K.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 20.f)), module, APFilter::G_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 35.f)), module, APFilter::D1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 50.f)), module, APFilter::D2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xMid, 65.f)), module, APFilter::D3_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xMid, 85.f)), module, APFilter::AUDIO_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xMid, 100.f)), module, APFilter::AUDIO1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xMid, 115.f)), module, APFilter::AUDIO2_OUTPUT));
	}
};


Model* modelAPFilter = createModel<APFilter, APFilterWidget>("APFilter");