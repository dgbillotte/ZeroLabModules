#include "plugin.hpp"

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

    // input, output, & delay variables
    float x_0=0, x_1=0, x_2=0,
        y_hpf_0=0, y_hpf_1=0, y_hpf_2=0,
        y_lpf_0=0, y_lpf_1=0, y_lpf_2=0;
    
    float TWO_PI = 2*M_PI;
    int sample_rate = 44100;
    
	FirstOrderBiQuad() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 0.f, 10000.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        x_0 = inputs[INPUT_INPUT].getVoltage();
        
        // shared coefficient calculations
        float theta = TWO_PI*freq/sample_rate;
        float gamma = cos(theta)/(1 + sin(theta));
        
        // hpf coefficients
        float a_0= (1 - gamma)/2;
        float a_1 = a_0;
        float a_2 = 0;
        float b_1 = -gamma;
        float b_2 = 0;
        
        // compute hpf output
        y_hpf_0 = a_0*x_0 + a_1*x_1 + a_2*x_2 - b_1*y_hpf_1 - b_2*y_hpf_2;
        
        // lpf coefficients
        a_0 = (1 + gamma)/2;
        a_1 = -a_0;
        a_2 = 0;
        b_1 = -gamma;
        b_2 = 0;
        
        // compute lpf output
        y_lpf_0 = a_0*x_0 + a_1*x_1 + a_2*x_2 - b_1*y_lpf_1 - b_2*y_lpf_2;
        
        // write the outputs
        outputs[HPF_OUTPUT].setVoltage(y_hpf_0);
        outputs[LPF_OUTPUT].setVoltage(y_lpf_0);
        
        // set delays
        x_2 = x_1;
        x_1 = x_0;
        y_hpf_2 = y_hpf_1;
        y_hpf_1 = y_hpf_0;
        y_lpf_2 = y_lpf_1;
        y_lpf_1 = y_lpf_0;
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
