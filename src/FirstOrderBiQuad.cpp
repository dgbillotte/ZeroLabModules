#include "plugin.hpp"
#include "lib/Filter.hpp"

struct FirstOrderBiQuad : Module {
	enum ParamIds {
		FREQKNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		HPF_OUTPUT,
        LPF_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
    
    OnePoleLPF* lpf = new OnePoleLPF();
    OnePoleHPF* hpf = new OnePoleHPF();
    
	FirstOrderBiQuad() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 0.f, 10000.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        float input = inputs[INPUT_INPUT].getVoltage();
        
        lpf->freq(freq);
        hpf->freq(freq);
        
        outputs[LPF_OUTPUT].setVoltage(lpf->process(input));
        outputs[HPF_OUTPUT].setVoltage(hpf->process(input));
	}
};


struct FirstOrderBiQuadWidget : ModuleWidget {
	FirstOrderBiQuadWidget(FirstOrderBiQuad* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FirstOrderBiQuad.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.262, 20.854)), module, FirstOrderBiQuad::FREQKNOB_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.262, 80)), module, FirstOrderBiQuad::INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 95)), module, FirstOrderBiQuad::HPF_OUTPUT));
        
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 110)), module, FirstOrderBiQuad::LPF_OUTPUT));
	}
};


Model* modelFirstOrderBiQuad = createModel<FirstOrderBiQuad, FirstOrderBiQuadWidget>("FirstOrderBiQuad");
