#include "plugin.hpp"


struct Resonator : Module {
	enum ParamIds {
		FREQKNOB_PARAM,
		Q_PARAM,
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

    float x_1=0, x_2=0, y_1=0, y_2=0;
    float TWO_PI = 2*M_PI;
    int sample_rate = 41000;
    
	Resonator() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 0.f, 10000.f, 0.f, "");
		configParam(Q_PARAM, 0.f, 11.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        float q = params[Q_PARAM].getValue();
        
        // calculate coefficients
        float theta = TWO_PI*freq/sample_rate;
        float bw = freq/q;
        float b_2 = exp(-TWO_PI*bw/sample_rate);
        float b_1 = -4*b_2*cos(theta)/(1+b_2);
        float a_0 = 1 - sqrt(b_2);
        float a_2 = -a_0;
        
        // compute the output
        float x_0 = inputs[INPUT_INPUT].getVoltage();
        float y_0 = a_0*x_0 + a_2*x_2 - b_1*y_1 - b_2*y_2;
        outputs[OUTPUT_OUTPUT].setVoltage(y_0);
        
        // update delay values
        x_2 = x_1;
        x_1 = x_0;
        y_2 = y_1;
        y_1 = y_0;
	}
};


struct ResonatorWidget : ModuleWidget {
	ResonatorWidget(Resonator* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Resonator.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.262, 20.854)), module, Resonator::FREQKNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.262, 40.317)), module, Resonator::Q_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.262, 90.829)), module, Resonator::INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 111.219)), module, Resonator::OUTPUT_OUTPUT));
	}
};


Model* modelResonator = createModel<Resonator, ResonatorWidget>("Resonator");
