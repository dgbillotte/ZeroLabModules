#include "plugin.hpp"


struct AnaLogic : Module {
	enum ParamIds {
		THRESH_PARAM,
		THRESH_CV_PARAM,
        LOGIC_MODEL_PARAM,
        SIGNALA_TYPE_PARAM,
        SIGNALB_TYPE_PARAM,
        EARTH_PARAM,
        WIND_PARAM,
        FIRE_PARAM,
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
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
    
    enum SignalType {
        ST_55,
        ST_Z10,
        ST_1010,
        ST_AUTO,
        ST_Unknown,
        ST_NUM_TYPES
    };
    
    float SCALE_FACTORS[3] = {5.f, 10.f, 10.f};
    
//    const int AUTO_SIGNAL_RATE = 44100; // how often to check the signal
//    int auto_signal_sample_count = 0;
//    int signal_a_type=ST_Unknown, signal_b_type=ST_Unknown;
    
    enum LogicModels {
        LM_MINMAX,
        LM_MULTADD,
        LM_BITLOGIC,
        LM_NUM_MODELS
    };

	AnaLogic() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESH_PARAM, -1.f, 1.f, 0.f, "Threshold");
		configParam(THRESH_CV_PARAM, 0.f, 1.f, 0.f, "Threshold CV");
        
        configParam(LOGIC_MODEL_PARAM, LM_MINMAX, LM_BITLOGIC, LM_MINMAX, "Logic model to use");
        configParam(SIGNALA_TYPE_PARAM, ST_55, ST_1010, ST_55, "Signal A Type");
        configParam(SIGNALB_TYPE_PARAM, ST_55, ST_1010, ST_55, "Signal B Type");
        
        configParam(EARTH_PARAM, 0.f, 1.f, 0.f, "Earth");
        configParam(WIND_PARAM, 0.f, 1.f, 0.f, "Wind");
        configParam(FIRE_PARAM, 0.f, 2.f, 1.f, "Fire");
	}
    
    

	void process(const ProcessArgs& args) override {
        // get input signals
        float a_in = inputs[IN1_INPUT].getVoltage();
        float b_in = inputs[IN2_INPUT].getVoltage();
        
        // scale input signals for the given signal type
        int signal_a_type = params[SIGNALA_TYPE_PARAM].getValue();
        a_in /= SCALE_FACTORS[signal_a_type];
        int signal_b_type = params[SIGNALB_TYPE_PARAM].getValue();
        b_in /= SCALE_FACTORS[signal_b_type];
        
        // calculate threshold for analog -> digital conversion
        float thresh = params[THRESH_PARAM].getValue();
        float thresh_cv_in = inputs[THRESH_CV_INPUT].getVoltage();
        float thresh_cv = params[THRESH_CV_PARAM].getValue();
        thresh += thresh_cv_in * thresh_cv;
    
        // calculate analog outs
        float analog_and_out, analog_or_out;
        int logic_model = params[LOGIC_MODEL_PARAM].getValue();
        if(logic_model == LM_MINMAX) {
            analog_and_out = std::min(a_in, b_in);
            analog_or_out = std::max(a_in, b_in);
            
        } else if(logic_model == LM_MULTADD) {
            analog_and_out = a_in * b_in;
            analog_or_out = (a_in + b_in)/2;
            
        } else {
            analog_and_out = (int)a_in & (int)b_in;
            analog_or_out = (int)a_in | (int)b_in;
        }
        analog_and_out = clamp1010(analog_and_out*5);
        analog_or_out = clamp1010(analog_or_out*5);
        
        // calculate digital outs
        float digital_and_out = (analog_and_out > thresh)*10;
        float digital_or_out = (analog_or_out > thresh)*10;
        
        // write individual outputs
        outputs[D_AND_OUTPUT].setVoltage(digital_and_out);
        outputs[D_OR_OUTPUT].setVoltage(digital_or_out);
        outputs[A_AND_OUTPUT].setVoltage(analog_and_out);
        outputs[A_OR_OUTPUT].setVoltage(analog_or_out);
        
        // calculate and write mix output
        float earth = params[EARTH_PARAM].getValue();
        float wind = params[WIND_PARAM].getValue();
        float fire = params[FIRE_PARAM].getValue();
        float d_earth = (earth*digital_and_out + (1-earth)*digital_or_out);
        float a_earth = (earth*analog_and_out + (1-earth)*analog_or_out);
        float mix_out = fire * (wind*d_earth + (1-wind)*a_earth);
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

        // get screwed
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // signal inputs and type controls
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(5.f, 10.f)), module, AnaLogic::SIGNALA_TYPE_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.f, 10.f)), module, AnaLogic::SIGNALB_TYPE_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.f, 20.f)), module, AnaLogic::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.f, 20.f)), module, AnaLogic::IN2_INPUT));
        
        // ADC threshold controls
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.f, 35.f)), module, AnaLogic::THRESH_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.f, 45.f)), module, AnaLogic::THRESH_CV_INPUT));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.f, 45.f)), module, AnaLogic::THRESH_CV_PARAM));
        
        // logic model switch
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(10.398f, 60.0f)), module, AnaLogic::LOGIC_MODEL_PARAM));
        
        // controls for the 3 elements
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.f, 75.f)), module, AnaLogic::EARTH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.f, 75.f)), module, AnaLogic::WIND_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.f, 85.f)), module, AnaLogic::FIRE_PARAM));
        
        // outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.f, 100.f)), module, AnaLogic::D_AND_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.f, 100.f)), module, AnaLogic::D_OR_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.f, 107.5f)), module, AnaLogic::MIX_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.f, 115.f)), module, AnaLogic::A_AND_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.f, 115.f)), module, AnaLogic::A_OR_OUTPUT));
        
	}
};


Model* modelAnaLogic = createModel<AnaLogic, AnaLogicWidget>("AnaLogic");
