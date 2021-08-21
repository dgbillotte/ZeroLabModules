#include "plugin.hpp"


struct Osc1 : Module {
	enum InputIds {
		VOCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		RAMP_OUTPUT,
		SIN_OUTPUT,
		SQR_OUTPUT,
		TRI_OUTPUT,
		NUM_OUTPUTS
	};
    enum ParamIds {
        FREQ_PARAM,
        RANGE_PARAM,
        POLARITY_PARAM,
        NUM_PARAMS
    };
	enum LightIds {
		NUM_LIGHTS
	};

	Osc1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_PARAM, 0, 10000, 440, "Freq(Hz)");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(POLARITY_PARAM, 0.f, 1.f, 0.f, "");
        
        
	}

    
    const float _PI = M_PI;
    const float _halfPI = _PI/2.f;
    const float _3halfPI = _PI + _halfPI;
    const float _twoPI = 2.f*_PI;
    
    float _phase_inc = 0.f;
    float _phase = 0.f;
    float _freq = 0.f;
    float tmin = 10.f;
    float tmax = 0.f;
    const int LOG_INTERVAL = 41000;
    int log_count = 0;
    
    
	void process(const ProcessArgs& args) override {
        float new_freq = params[FREQ_PARAM].getValue();
        if(_phase_inc == 0 || (_freq != new_freq)) {
            _freq = new_freq;
            _phase_inc = _twoPI * _freq / args.sampleRate;
        }
        
        float sin_out = sin(_phase); // this is [-1..1]
        if(outputs[SIN_OUTPUT].isConnected()) {
            outputs[SIN_OUTPUT].setVoltage(5.f*sin_out);
        }
        
        if(outputs[TRI_OUTPUT].isConnected()) {
            float tri_out = _phase * 2.f / M_PI;
            float sqr_out = -1.f;
            
            if(_phase < M_PI/2.f) {
                //tri_out = 0.f + tri_out;
            } else if(_phase < _3halfPI) {
                tri_out = 2.f - tri_out;
                sqr_out = 1.f;
            } else {
                tri_out = -4.f + tri_out;
            }
            
            outputs[TRI_OUTPUT].setVoltage(5.f*tri_out);
            
        }
               
        // first attempt at tri, was wrong, but interesting
//        if(outputs[TRI_OUTPUT].isConnected()) {
//            float tri_out = _phase * 2 / _PI;
//            if(_phase < M_PI) {
//                tri_out = 0.f + tri_out;
//            } else {
//                tri_out = 0.f - tri_out;
//            }
//            outputs[TRI_OUTPUT].setVoltage(5*tri_out);
//        }
        
        if(outputs[SQR_OUTPUT].isConnected()) {
            float sqr_out = (sin_out > 0.f) ? 1.f : -1.f;
            
            
            outputs[SQR_OUTPUT].setVoltage(5.f*sqr_out);
        }

        
        if(outputs[RAMP_OUTPUT].isConnected()) {
            outputs[RAMP_OUTPUT].setVoltage(5.f*(_phase/M_PI - 1.f));
        }
        
        _phase += _phase_inc;
        if(_phase > _twoPI)
            _phase -= _twoPI;
	}
};


struct Osc1Widget : ModuleWidget {
	Osc1Widget(Osc1* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Osc1.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.092, 21.907)), module, Osc1::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(9.62, 74.753)), module, Osc1::RANGE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.033, 74.981)), module, Osc1::POLARITY_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.967, 48.537)), module, Osc1::VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.427, 103.902)), module, Osc1::RAMP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.913, 104.365)), module, Osc1::SIN_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.813, 118.608)), module, Osc1::SQR_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.024, 119.272)), module, Osc1::TRI_OUTPUT));
	}
};


Model* modelOsc1 = createModel<Osc1, Osc1Widget>("Osc1");
