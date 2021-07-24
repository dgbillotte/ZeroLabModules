#include "plugin.hpp"


struct TrackHold : Module {
	enum ParamIds {
		KNOB1_PARAM,
		KNOB2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT1A_INPUT,
		INPUT1B_INPUT,
		INPUT2A_INPUT,
		INPUT2B_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT1_OUTPUT,
		OUTPUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    int last_gate_high1 = false;
    float sample1 = 0;
    int last_gate_high2 = false;
    float sample2 = 0;
    
	TrackHold() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(KNOB1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB2_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
        // Channel #1
        float track_or_sample = params[KNOB1_PARAM].getValue();
        int gate_high = inputs[INPUT1A_INPUT].getVoltage() > 0 ? true : false;
        float input = inputs[INPUT1B_INPUT].getVoltage();
        
        if((track_or_sample < 0.5 && (gate_high || last_gate_high1)) ||
           (track_or_sample >= 0.5 && (gate_high && !last_gate_high1))) {
            sample1 = input;
        }
        
        last_gate_high1 = gate_high;
        outputs[OUTPUT1_OUTPUT].setVoltage(sample1);
        
        // Channel #2
        track_or_sample = params[KNOB2_PARAM].getValue();
        gate_high = inputs[INPUT2A_INPUT].getVoltage() > 0 ? true : false;
        input = inputs[INPUT2B_INPUT].getVoltage();
        
        if((track_or_sample < 0.5 && (gate_high || last_gate_high2)) ||
           (track_or_sample >= 0.5 && (gate_high && !last_gate_high2))) {
            sample2 = input;
        }
        
        last_gate_high2 = gate_high;
        outputs[OUTPUT2_OUTPUT].setVoltage(sample2);
	}
    
};


struct TrackHoldWidget : ModuleWidget {
	TrackHoldWidget(TrackHold* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TrackHold.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.514, 16.567)), module, TrackHold::KNOB1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.514, 71.192)), module, TrackHold::KNOB2_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.514, 28.114)), module, TrackHold::INPUT1A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.514, 39.66)), module, TrackHold::INPUT1B_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.514, 82.738)), module, TrackHold::INPUT2A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.514, 94.285)), module, TrackHold::INPUT2B_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.514, 51.207)), module, TrackHold::OUTPUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.514, 105.832)), module, TrackHold::OUTPUT2_OUTPUT));
	}
};


Model* modelTrackHold = createModel<TrackHold, TrackHoldWidget>("TrackHold");
