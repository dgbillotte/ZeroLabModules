#include "plugin.hpp"


struct AnaLogic : Module {
	enum ParamIds {
		THRESH_PARAM,
		THRESH_CV_PARAM,
        LOGIC_MODEL_PARAM,
        YIN_PARAM,
        YANG_PARAM,
//        POLAR_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		THRESH_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		D_AND_OUTPUT,
        D_OR_OUTPUT,
		A_AND_OUTPUT,
        A_OR_OUTPUT,
        MIX_OUTPUT,
        THRESH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
    
    enum LogicModels {
        LM_MINMAX,
        LM_MULTADD,
        LM_NUM_MODELS
    };
    
//    enum Polarity {
//        UNIPOLAR, // 0-10V
//        BIPOLAR   // -5-5V
//    };

	AnaLogic() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESH_PARAM, -1.f, 1.f, 0.f, "Threshold");
		configParam(THRESH_CV_PARAM, 0.f, 1.f, 0.f, "Threshold CV");
        configParam(LOGIC_MODEL_PARAM, LM_MINMAX, LM_MULTADD, LM_MINMAX, "Logic model to use");
        configParam(YIN_PARAM, 0.f, 1.f, 0.f, "Yin");
        configParam(YANG_PARAM, 0.f, 1.f, 0.f, "Yang");
//        configParam(MODE_PARAM, UNIPOLAR, BIPOLAR, UNIPOLAR, "uni/bi-polar");
	}
    
//    int polarity = UNIPOLAR;

	void process(const ProcessArgs& args) override {
        // get input signals and scale them by 1/5
        float a_in = inputs[IN1_INPUT].getVoltage()/5;
        float b_in = inputs[IN2_INPUT].getVoltage()/5;
        
        
        float thresh_cv_in = inputs[THRESH_CV_INPUT].getVoltage();
        
        float thresh = params[THRESH_PARAM].getValue();
        float thresh_cv = params[THRESH_CV_PARAM].getValue();
        thresh += thresh_cv_in * thresh_cv;
        
        
        // the "max" operation
        // note: vs just using std::max() on the
        // raw inputs, this is cleaner, but more
        // boring and has large zero spots
//        float analog_out = a_in;
//        if(abs(b_in) > abs(a_in))
//            analog_out = b_in;
        
        float analog_and_out, analog_or_out;
        if(params[LOGIC_MODEL_PARAM].getValue() == LM_MINMAX) {
            analog_and_out = std::min(a_in, b_in);
            analog_or_out = std::max(a_in, b_in);
        } else {
            analog_and_out = a_in * b_in;
            analog_or_out = (a_in + b_in)/2;
        }
        analog_and_out = clamp1010(analog_and_out*5);
        analog_or_out = clamp1010(analog_or_out*5);
        
        
        float digital_and_out = (analog_and_out > thresh)*10;
        float digital_or_out = (analog_or_out > thresh)*10;
        
//        outputs[THRESH_OUTPUT].setVoltage(clamp1010(thresh));
        outputs[D_AND_OUTPUT].setVoltage(digital_and_out);
        outputs[D_OR_OUTPUT].setVoltage(digital_or_out);
        outputs[A_AND_OUTPUT].setVoltage(analog_and_out);
        outputs[A_OR_OUTPUT].setVoltage(analog_or_out);
        
        float yin = params[YIN_PARAM].getValue();
        float yang = params[YANG_PARAM].getValue();
        
        float d_yin = (yin*digital_and_out + (1-yin)*digital_or_out) / 2;
        float a_yin = (yin*analog_and_out + (1-yin)*analog_or_out) / 2;
        float mix_out = (yang*d_yin + (1-yang)*a_yin)/2;
        outputs[MIX_OUTPUT].setVoltage(mix_out);
	}
    
    float abs(float n);
    float clamp55(float n);
    float clamp1010(float n);
    float clampZ10(float n);
};

//inline float AnaLogic::max(float a, float b) {
//
//}
inline float AnaLogic::abs(float n) {
    return n >= 0 ? n : -n;
}

inline float AnaLogic::clamp55(float n) {
    return clamp(n, -5.f, 5.f);
}

inline float AnaLogic::clamp1010(float n) {
    return clamp(n, -10.f, 10.f);
}

inline float AnaLogic::clampZ10(float n) {
    return clamp(n, 0.f, 10.f);
}




struct AnaLogicWidget : ModuleWidget {
	AnaLogicWidget(AnaLogic* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AnaLogic.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

//		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.398, 14.675)), module, AnaLogic::IN1_INPUT));
//		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.398, 27.361)), module, AnaLogic::IN2_INPUT));
        
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 10.0)), module, AnaLogic::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.0, 10.0)), module, AnaLogic::IN2_INPUT));
        
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.398, 25.0)), module, AnaLogic::THRESH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.398, 40.0)), module, AnaLogic::THRESH_CV_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.398, 50.0)), module, AnaLogic::THRESH_CV_INPUT));
        
        // added manually
//        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.398, 80.0)), module, AnaLogic::THRESH_OUTPUT));
        
        addParam(createParamCentered<CKSS>(mm2px(Vec(10.398, 65)), module, AnaLogic::LOGIC_MODEL_PARAM));
        
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.0, 80.0)), module, AnaLogic::YIN_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 80.0)), module, AnaLogic::YANG_PARAM));
        
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.0, 95.0)), module, AnaLogic::D_AND_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.0, 95.0)), module, AnaLogic::D_OR_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.0, 115.0)), module, AnaLogic::A_AND_OUTPUT));
        
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.0, 115.0)), module, AnaLogic::A_OR_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.0, 100.0)), module, AnaLogic::MIX_OUTPUT));
	}
};


Model* modelAnaLogic = createModel<AnaLogic, AnaLogicWidget>("AnaLogic");
