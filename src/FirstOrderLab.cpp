#include "plugin.hpp"
#include <math.h>
#include "Filter.hpp"


struct FirstOrderLab : Module {
	enum ParamIds {
		FREQKNOB_PARAM,
        MODE_PARAM,
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
    
    enum Modes {
        LPF_MODE,
        HPF_MODE
    };

    float y_1 = 5;
    Filter* lpf = new OnePoleLPF();
    
	FirstOrderLab() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 0.f, 10000.f, 10000.f, "Frequency range: 0-10k Hz");
        configParam(MODE_PARAM, LPF_MODE, HPF_MODE, LPF_MODE, "LPF or HPF");
	}

    void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        lpf->freq(freq);
        float input = inputs[INPUT_INPUT].getVoltage();
        float output = lpf->process(input);
        outputs[OUTPUT_OUTPUT].setVoltage(output);
    }
};


struct FirstOrderLabWidget : ModuleWidget {
	FirstOrderLabWidget(FirstOrderLab* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FirstOrderLab.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.262, 20.854)), module, FirstOrderLab::FREQKNOB_PARAM));
        
        addParam(createParamCentered<CKSS>(mm2px(Vec(10.262, 40)), module,
            FirstOrderLab::MODE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.262, 90.829)), module, FirstOrderLab::INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 111.219)), module, FirstOrderLab::OUTPUT_OUTPUT));
	}
};


Model* modelFirstOrderLab = createModel<FirstOrderLab, FirstOrderLabWidget>("FirstOrderLab");
