#include "plugin.hpp"


struct AnaLogic2 : Module {
    enum InputIds {
        A_INPUT,
        B_INPUT,
        MM_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        PRE_A_OUTPUT,
        PRE_B_OUTPUT,
        D_AND_OUTPUT,
        D_OR_OUTPUT,
        A_AND_OUTPUT,
        A_OR_OUTPUT,
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum ParamIds {
        INV_A_PARAM,
        INV_B_PARAM,
        SHIFT_A_PARAM,
        SHIFT_B_PARAM,
        GAIN_A_PARAM,
        GAIN_B_PARAM,
        MM_PARAM,
        EARTH_PARAM,
        WIND_PARAM,
        FIRE_PARAM,
        NUM_PARAMS
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
    
//    float SCALE_FACTORS[3] = {5.f, 10.f, 10.f};
    
    enum LogicModels {
        LM_MIN_MAX,
        LM_MULT_ADD,
        LM_BIT_LOGIC,
        LM_EXPR_LOGIC,
        LM_PASS_THRU,
        LM_NUM_MODELS
    };

	AnaLogic2() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(INV_A_PARAM, 0, 1, 0, "Invert A");
        configParam(INV_B_PARAM, 0, 1, 0, "Invert B");
        configParam(SHIFT_A_PARAM, -10.f, 10.f, 0.f, "+/- 10V DC A");
        configParam(SHIFT_B_PARAM, -10.f, 10.f, 0.f, "+/- 10V DC B");
        configParam(GAIN_A_PARAM, 0.f, 2.f, 1.f, "Gain A");
        configParam(GAIN_B_PARAM, 0.f, 2.f, 1.f, "Gain B");
        
        configParam(MM_PARAM, LM_MIN_MAX, LM_PASS_THRU, LM_MIN_MAX, "Math Model");
        
        configParam(EARTH_PARAM, 0.f, 1.f, 0.5f, "Earth");
        configParam(WIND_PARAM, 0.f, 1.f, 0.5f, "Wind");
        configParam(FIRE_PARAM, 0.f, 2.f, 1.f, "Fire");
	}  

	void process(const ProcessArgs& args) override {
        
        // get input signals
        float a_in = inputs[A_INPUT].getVoltage();
        float b_in = inputs[B_INPUT].getVoltage();
        
        // shift, scale, & invert? the inputs
        float gain_a = (params[INV_A_PARAM].getValue() ? -1 : 1) * params[GAIN_A_PARAM].getValue();
        
        a_in = (a_in * gain_a) + params[SHIFT_A_PARAM].getValue();
        
        float gain_b = (params[INV_B_PARAM].getValue() ? -1 : 1) * params[GAIN_B_PARAM].getValue();
        
        b_in = (b_in * gain_b) + params[SHIFT_B_PARAM].getValue();
        
        // write the pre-processed signals out
        outputs[PRE_A_OUTPUT].setVoltage(clamp1010(a_in));
        outputs[PRE_B_OUTPUT].setVoltage(clamp1010(b_in));
        
        
        // this got accidentally left out in the redesign and
        // I'm wondering if that isn't a good thing...
        
        // calculate threshold for analog -> digital conversion
//        float thresh = params[THRESH_PARAM].getValue();
//        float thresh_cv_in = inputs[THRESH_CV_INPUT].getVoltage();
//        float thresh_cv = params[THRESH_CV_PARAM].getValue();
//        thresh += thresh_cv_in * thresh_cv;
        float thresh = 0.f;
    
        // calculate analog outs
        float analog_and_out = a_in, analog_or_out = b_in;
        int logic_model = params[MM_PARAM].getValue();
        
        if(logic_model == LM_MIN_MAX) {
            analog_and_out = min(a_in, b_in);
            analog_or_out = max(a_in, b_in);
//            std::cout << "Math model: min/max" << std::endl;
            
        } else if(logic_model == LM_MULT_ADD) {
            analog_and_out = a_in * b_in;
            analog_or_out = (a_in + b_in)/2;
//            std::cout << "Math model: mult/add" << std::endl;
            
        } else if(logic_model == LM_BIT_LOGIC) {
            analog_and_out = (int)a_in & (int)b_in;
            analog_or_out = (int)a_in | (int)b_in;
//            std::cout << "Math model: bit logic" << std::endl;
            
        } else if(logic_model == LM_EXPR_LOGIC) {
            analog_and_out = (int)a_in && (int)b_in;
            analog_or_out = (int)a_in || (int)b_in;
//            std::cout << "Math model: expr logic" << std::endl;
            
        } //else {
//            analog_and_out = a_in;
//            analog_or_out = b_in;
//            std::cout << "Math model: pass-thru" << std::endl;
//        }
        
        analog_and_out = clamp55(analog_and_out*5);
        analog_or_out = clamp55(analog_or_out*5);
        
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
    float min(float a, float b);
    float max(float a, float b);
    float clamp55(float n);
    float clamp1010(float n);
    float clampZ10(float n);
};

inline float AnaLogic2::abs(float n) { return n >= 0 ? n : -n; }
inline float AnaLogic2::min(float a, float b) { return a < b ? a : b; }
inline float AnaLogic2::max(float a, float b) { return a > b ? a : b; }
inline float AnaLogic2::clamp55(float n) { return clamp(n, -5.f, 5.f); }
inline float AnaLogic2::clamp1010(float n) { return clamp(n, -10.f, 10.f); }
inline float AnaLogic2::clampZ10(float n) { return clamp(n, 0.f, 10.f); }


struct AnaLogic2Widget : ModuleWidget {
	// module dimensions and handy guides
	float width = 50.80;
	float midX = width/2;
	float height = 128.5;
	float _8th = width/8;
	float _7_8th = width-_8th;
    float gutter = 5.f;

    AnaLogic2Widget(AnaLogic2* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AnaLogic2000.svg")));

        // get screwed
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        float rowY = 25.f;
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_8th+gutter, rowY)), module, AnaLogic2::SHIFT_A_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(midX, rowY)), module, AnaLogic2::INV_A_PARAM));
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, AnaLogic2::SHIFT_B_PARAM));

        rowY = 40.f;
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_8th+gutter, rowY)), module, AnaLogic2::GAIN_A_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(midX, rowY)), module, AnaLogic2::INV_B_PARAM));
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, AnaLogic2::GAIN_B_PARAM));

       
        rowY = 60.f;
        // addParam(createParamCentered<CKSSThree>(mm2px(Vec(21.373, rowY)), module, AnaLogic2::MM_PARAM));
        addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(Vec(midX, rowY)), module, AnaLogic2::MM_PARAM));

        rowY = 75;
        addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(_8th+gutter, rowY)), module, AnaLogic2::EARTH_PARAM));
        addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(midX, rowY+10)), module, AnaLogic2::WIND_PARAM));
        addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, AnaLogic2::FIRE_PARAM));

//        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.181, 62.881)), module, AnaLogic2::MM_CV_INPUT));



        // jacks
        rowY = 98.f;
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_8th, rowY)), module, AnaLogic2::A_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(3*_8th, rowY)), module, AnaLogic2::PRE_A_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5*_8th, rowY)), module, AnaLogic2::A_AND_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(_7_8th, rowY)), module, AnaLogic2::D_AND_OUTPUT));

        rowY += 15.f;
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(_8th, rowY)), module, AnaLogic2::B_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(3*_8th, rowY)), module, AnaLogic2::PRE_B_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5*_8th, rowY)), module, AnaLogic2::A_OR_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(_7_8th, rowY)), module, AnaLogic2::D_OR_OUTPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6*_8th, rowY-7.5)), module, AnaLogic2::MIX_OUTPUT));


    }
};




Model* modelAnaLogic2 = createModel<AnaLogic2, AnaLogic2Widget>("AnaLogic2");
