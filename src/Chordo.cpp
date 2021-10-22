#include "plugin.hpp"
#include "lib/Components.hpp"

struct Chordo : Module {
	enum ParamIds {
		// MODE_PARAM,
		FREQ_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		VOCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CHANGE_TRIGGER_OUTPUT,
		ROOT_VOCT_OUTPUT,
		THIRD_VOCT_OUTPUT,
		FIFTH_VOCT_OUTPUT,
		SIX_7TH_VOCT_OUTPUT,
		ROOT_X2_VOCT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const float E2 = 82.41f;
	float _lastRoot = 0.f;
	rack::dsp::PulseGenerator _chordChange;

	Chordo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		// configParam(MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FREQ_PARAM, 82.41f, 220.f, 0.f, "Root Note");
	}

	void process(const ProcessArgs& args) override {
		// float root = params[FREQ_PARAM].getValue();
		float vOct = inputs[VOCT_INPUT].getVoltage();
		float root = _votof(vOct);
	
		float third = root * 1.26f;
		float fifth = root * 1.498f;
		float seventh = root * 1.888f;

		outputs[ROOT_VOCT_OUTPUT].setVoltage(_ftovo(root));
		outputs[THIRD_VOCT_OUTPUT].setVoltage(_ftovo(third));
		outputs[FIFTH_VOCT_OUTPUT].setVoltage(_ftovo(fifth));
		outputs[SIX_7TH_VOCT_OUTPUT].setVoltage(_ftovo(seventh));
		outputs[ROOT_X2_VOCT_OUTPUT].setVoltage(_ftovo(root*2));

		// set the changed trigger as needed
		if(root != _lastRoot) {
			_chordChange.trigger();
			_lastRoot = root;
		}
		int triggerOut = (_chordChange.process(args.sampleTime)) ? 10.f : 0.f;
		outputs[CHANGE_TRIGGER_OUTPUT].setVoltage(triggerOut);
	}

	float _ftovo(float freq) {
		return log2(freq / E2);
	}

	float _votof(float vOct) {
		return E2 * pow(2.f, vOct);
	}
};


struct ChordoWidget : ModuleWidget {

	float width = 20.32f;
	float midX = width/2.f;

	ChordoWidget(Chordo* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Chordo.svg")));

		// screws
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 10.f;
		float rowY = 18.f;
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Chordo::MODE_PARAM));
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(midX, rowY)), module, Chordo::FREQ_PARAM));

		rowY += rowInc;
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::VOCT_INPUT));

		rowY += rowInc*2;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::CHANGE_TRIGGER_OUTPUT));

		rowY += rowInc*2;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::ROOT_VOCT_OUTPUT));

		rowY += rowInc;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::THIRD_VOCT_OUTPUT));

		rowY += rowInc;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::FIFTH_VOCT_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::SIX_7TH_VOCT_OUTPUT));

		rowY += rowInc;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(midX, rowY)), module, Chordo::ROOT_X2_VOCT_OUTPUT));
	}
};


Model* modelChordo = createModel<Chordo, ChordoWidget>("Chordo");