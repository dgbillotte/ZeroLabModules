#include "plugin.hpp"
#include "lib/Components.hpp"
/*
 * This is a "signal combiner" that is based off of the idea
 * of boolean logic as it would work with digital signals. It 
 * takes 2 signals (digital or analog) and combines them with
 * using one of 4 different logic-models. Each logic model is
 * a computation that would do boolean logic with digital signals
 * but does different weird things when you throw analog signals
 * into the mix.
 * 
 * The first section of the module allows you to shift/scale/invert
 * each of the A/B inputs. There are outputs for these value values.
 * 
 * The next section uses one or two logic models to evaluate an
 * output. If the model param is right on one value, that model
 * is used to generate the output. If the model param is in-between
 * two values, both models are evaluated and the output is
 * interpolated between the two.
 * 
 * While I did not set out to make a hardware clone, I have to
 * give credit to Eli Pechman of Mystic Circuits for the inspiration
 * for what it is. I heard him talk about his Ana module (wondering
 * why he named it that...) and built one of his Spectra Mirror
 * kits, which is super fun to play with. As I was working on this
 * module and toying with the name "Analog Logic", Eli's "Ana" all
 * of a sudden made sense... Thanks Eli!
 */



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
        WATER_PARAM,
        FIRE_PARAM,
        NUM_PARAMS
    };
	enum LightIds {
        LM1_LIGHT,
        LM2_LIGHT,
        LM3_LIGHT,
        LM4_LIGHT,
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
    enum LogicModels {
        LM_MIN_MAX,
        LM_BIT_LOGIC,
        LM_MULT_ADD,
        LM_EXPR_LOGIC,
        LM_NUM_MODELS
    };

    typedef std::function<std::pair<float,float>(float,float)> LogicF;

    const float SNAP_THRESHOLD = 0.1f;
    LogicF logicFunctions[LM_NUM_MODELS];
    float _analogAndOut;
    float _analogOrOut;

	AnaLogic2() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(INV_A_PARAM, 0, 1, 0, "Invert A");
        configParam(INV_B_PARAM, 0, 1, 0, "Invert B");
        configParam(SHIFT_A_PARAM, -10.f, 10.f, 0.f, "+/- 10V DC A");
        configParam(SHIFT_B_PARAM, -10.f, 10.f, 0.f, "+/- 10V DC B");
        configParam(GAIN_A_PARAM, 0.f, 2.f, 1.f, "Gain A");
        configParam(GAIN_B_PARAM, 0.f, 2.f, 1.f, "Gain B");
        
        configParam(MM_PARAM, LM_MIN_MAX, LM_NUM_MODELS-0.0001f, LM_MIN_MAX, "Math Model");
        
        configParam(EARTH_PARAM, 0.f, 1.f, 0.5f, "Earth");
        configParam(WATER_PARAM, 0.f, 1.f, 0.5f, "Water");
        configParam(FIRE_PARAM, 0.f, 2.f, 1.f, "Fire");
        
        lights[LM1_LIGHT].setBrightness(0.f);
        lights[LM2_LIGHT].setBrightness(0.f);
        lights[LM3_LIGHT].setBrightness(0.f);
        lights[LM4_LIGHT].setBrightness(0.f);

        // this is the core logic for each logic-model
        logicFunctions[LM_MIN_MAX] = [&](float a, float b) {
            return std::make_pair(min(a, b), max(a, b));
        };
        logicFunctions[LM_MULT_ADD] = [&](float a, float b) {
            // this produces halved amplitudes. w/o it, it gets too big
            // perhaps, just clamp it...
            return std::make_pair(a * b, (a+b)/2);
        };
        logicFunctions[LM_BIT_LOGIC] = [&](float a, float b) {
            // NOTE: it might be interesting to capture the sign
            // and then do the operation with unsigned, then resign it...
            return std::make_pair((int)a & (int)b, (int)a | (int)b);
        };
        logicFunctions[LM_EXPR_LOGIC] = [&](float a, float b) {
            return std::make_pair((int)a && (int)b, (int)a || (int)b);
        };
	}  

    int _count = 0;
	void process(const ProcessArgs& args) override;
    std::pair<float, float> _runLogicModel(float logicModel, float aIn, float bIn);
    void _setLights(int light1);
    void _setLights(int light1, int light2, float brightness);
    float abs(float n);
    float min(float a, float b);
    float max(float a, float b);
    float clamp55(float n);
    float clamp1010(float n);
    float clampZ10(float n);
};

void AnaLogic2::_setLights(int light) {
    for(int i=0; i < NUM_LIGHTS; i++) {
        lights[i].setBrightness((i == light) ? 1.f : 0.f);
    }
}

void AnaLogic2::_setLights(int light1, int light2, float brightness) {
    for(int i=0; i < NUM_LIGHTS; i++) {
        float b = (i == light1) ? brightness : (i == light2) ? 1.f-brightness : 0.f;
        lights[i].setBrightness(b);    
    }
}

void AnaLogic2::process(const ProcessArgs& args) {
    _count++; 
    
    // get input signals
    float aIn = inputs[A_INPUT].getVoltage();
    float bIn = inputs[B_INPUT].getVoltage();
    
    // shift, scale, & invert? the inputs
    float gainA = (params[INV_A_PARAM].getValue() ? -1 : 1) * params[GAIN_A_PARAM].getValue();
    
    aIn = (aIn * gainA) + params[SHIFT_A_PARAM].getValue();
    
    float gainB = (params[INV_B_PARAM].getValue() ? -1 : 1) * params[GAIN_B_PARAM].getValue();
    
    bIn = (bIn * gainB) + params[SHIFT_B_PARAM].getValue();
    
    // write the pre-processed signals out
    outputs[PRE_A_OUTPUT].setVoltage(clamp1010(aIn));
    outputs[PRE_B_OUTPUT].setVoltage(clamp1010(bIn));
    
    
    // this used to be a param, but it didn't seem that useful
    float thresh = 0.f;

    // calculate analog outs
    float logicModel = params[MM_PARAM].getValue();
    float analogAndOut;
    float analogOrOut;
    std::tie(analogAndOut, analogOrOut) = _runLogicModel(logicModel, aIn, bIn);
    analogAndOut = clamp55(analogAndOut*5);
    analogOrOut = clamp55(analogOrOut*5);
    
    // calculate digital outs
    float digitalAndOut = (analogAndOut > thresh)*10;
    float digitalOrOut = (analogOrOut > thresh)*10;
    
    // write individual outputs
    outputs[D_AND_OUTPUT].setVoltage(digitalAndOut);
    outputs[D_OR_OUTPUT].setVoltage(digitalOrOut);
    outputs[A_AND_OUTPUT].setVoltage(analogAndOut);
    outputs[A_OR_OUTPUT].setVoltage(analogOrOut);
    
    // calculate and write mix output
    float earth = params[EARTH_PARAM].getValue();
    float water = params[WATER_PARAM].getValue();
    float fire = params[FIRE_PARAM].getValue();
    float dEarth = (earth*digitalAndOut + (1-earth)*digitalOrOut);
    float aEarth = (earth*analogAndOut + (1-earth)*analogOrOut);
    float mixOut = fire * (water*dEarth + (1-water)*aEarth);
    outputs[MIX_OUTPUT].setVoltage(mixOut);
}
    

inline float AnaLogic2::abs(float n) { return n >= 0 ? n : -n; }
inline float AnaLogic2::min(float a, float b) { return a < b ? a : b; }
inline float AnaLogic2::max(float a, float b) { return a > b ? a : b; }
inline float AnaLogic2::clamp55(float n) { return clamp(n, -5.f, 5.f); }
inline float AnaLogic2::clamp1010(float n) { return clamp(n, -10.f, 10.f); }
inline float AnaLogic2::clampZ10(float n) { return clamp(n, 0.f, 10.f); }
inline std::pair<float, float> AnaLogic2::_runLogicModel(float logicModel, float aIn, float bIn) {
    logicModel = (logicModel < 5.f) ? logicModel : 0.f;
    int modelA = floor(logicModel);
    float frac = logicModel - modelA;

    // make it snap to nearby model
    if(frac < AnaLogic2::SNAP_THRESHOLD) {
        // run the single logic model
        auto pair = logicFunctions[modelA](aIn, bIn);
        _setLights(modelA);
        return pair;
    }

    // run two logic models and then mix them by 
    // the fraction in-between them.
    int modelB = (modelA+1 < LM_NUM_MODELS) ? (modelA + 1) : 0;
    auto pair1 = logicFunctions[modelA](aIn, bIn);
    auto pair2 = logicFunctions[modelB](aIn, bIn);
    float v1 = (1-frac)*pair1.first + frac*pair2.first;
    float v2 = (1-frac)*pair1.second + frac*pair2.second;
    _setLights(modelA, modelB, 1.f-frac);
    return std::make_pair(v1, v2);
}


struct AnaLogic2Widget : ModuleWidget {
	// module dimensions and handy guides
	float width = 50.80;
	float midX = width/2;
	float height = 128.5;
    float midY = height/2;
	float _8th = width/8;
	float _7_8th = width-_8th;
    float gutter = 5.f;

    AnaLogic2Widget(AnaLogic2* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AnaLogic2000.svg")));

        // screws
        addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        float rowY = 18.f;
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_8th+gutter, rowY)), module, AnaLogic2::SHIFT_A_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(midX, rowY)), module, AnaLogic2::INV_A_PARAM));
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, AnaLogic2::SHIFT_B_PARAM));

        rowY = 35.f;
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_8th+gutter, rowY)), module, AnaLogic2::GAIN_A_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(midX, rowY)), module, AnaLogic2::INV_B_PARAM));
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(_7_8th-gutter, rowY)), module, AnaLogic2::GAIN_B_PARAM));

        // lay out the mode and earth,water,fire params in a circle, cause....?
        Vec redKnob = Vec(midX, 66.f);
        float kRadius = 18.f;
        float kx = kRadius*0.866f; // cos(30)
        float ky = kRadius*0.5f; // sin(30)
        float lRadius = 11.f;
 
        float rotation = 0.13f;
        Vec earthPos = rotate(Vec(redKnob.x - kx, redKnob.y - ky), redKnob, rotation);
        Vec waterPos = rotate(Vec(redKnob.x + kx, redKnob.y - ky), redKnob, rotation);
        Vec firePos = rotate(Vec(redKnob.x, redKnob.y + kRadius), redKnob, rotation);
        addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(redKnob), module, AnaLogic2::MM_PARAM));
        addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(firePos), module, AnaLogic2::FIRE_PARAM));
        addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(earthPos), module, AnaLogic2::EARTH_PARAM));
        addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(waterPos), module, AnaLogic2::WATER_PARAM));

        rotation = -0.08f;
        Vec logic1Pos = rotate(Vec(redKnob.x, redKnob.y + lRadius), redKnob, rotation);
        Vec logic2Pos = rotate(Vec(redKnob.x - lRadius, redKnob.y), redKnob, rotation);
        Vec logic3Pos = rotate(Vec(redKnob.x, redKnob.y - lRadius), redKnob, rotation);
        Vec logic4Pos = rotate(Vec(redKnob.x + lRadius, redKnob.y), redKnob, rotation);
        addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(logic1Pos), module, AnaLogic2::LM1_LIGHT));
        addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(logic2Pos), module, AnaLogic2::LM2_LIGHT));
        addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(logic3Pos), module, AnaLogic2::LM3_LIGHT));
        addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(logic4Pos), module, AnaLogic2::LM4_LIGHT));
        
     
        // jacks
        rowY = 100.f;
        addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, AnaLogic2::A_INPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(3*_8th, rowY)), module, AnaLogic2::PRE_A_OUTPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(5*_8th, rowY)), module, AnaLogic2::A_AND_OUTPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(_7_8th, rowY)), module, AnaLogic2::D_AND_OUTPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(_7_8th, rowY-13)), module, AnaLogic2::MIX_OUTPUT));

        rowY = 113.f;
        addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, AnaLogic2::B_INPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(3*_8th, rowY)), module, AnaLogic2::PRE_B_OUTPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(5*_8th, rowY)), module, AnaLogic2::A_OR_OUTPUT));
        addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(_7_8th, rowY)), module, AnaLogic2::D_OR_OUTPUT));

    }


    // Thanks to Nils Pipenbrinck and twe4ked on stackoverflow for this code
    Vec rotate(Vec point, Vec origin, float degrees) {
        // translate the origin out
        float px = point.x - origin.x;
        float py = point.y - origin.y;

        // rotate about 0,0
        float s = sin(degrees);
        float c = cos(degrees);
        float x = px * c - py * s;
        float y = px * s + py * c;

        // translate back to orig position
        x += origin.x;
        y += origin.y;

        return Vec(x,y);
    }
};

Model* modelAnaLogic2 = createModel<AnaLogic2, AnaLogic2Widget>("AnaLogic2");
