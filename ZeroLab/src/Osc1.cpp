#include "plugin.hpp"
//#include "AudioLib.hpp"
#include "Osc.hpp"

/*
 
 For the Osc class have two different interfaces
 - one shot: get the next sample from the osc and advance it
    float out = osc.nextSine();
 
 - multiple: get whichever samples wanted, then advance it
    float sine = osc.sine();
    float sqr = osc.square();
    osc.next();
 
 */
//class SineOsc {
//protected:
//    float _freq = 0.f;
//    float _phaseInc = 0.f;
//    float _phase = 0.f;
//    int _sampleRate = 0;
//
//public:
//    void setFreq(float freq) {
//        if(_freq != freq) {
//            _freq = freq;
//            setPhaseInc();
//        }
//    }
//
//    void setSampleRate(int sampleRate) {
//        if(_sampleRate != sampleRate) {
//            _sampleRate = sampleRate;
//            setPhaseInc();
//        }
//    }
//
//    void setPhaseParams(float freq, int sampleRate) {
//        _freq = freq;
//        _sampleRate = sampleRate;
//        setPhaseInc();
//    }
//
//    float next() {
//        float out = sin(_phase);
//
//        _phase += _phaseInc;
//        if(_phase > _twoPI)
//            _phase -= _twoPI;
//
//        return out;
//    }
//
//protected:
//    void setPhaseInc() {
//        _phaseInc = _twoPI * _freq / _sampleRate;
//    }
//};


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
        WHA_OUTPUT,
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

    float _phase_inc = 0.f;
    float _phase = 0.f;
    float _freq = 0.f;
    
//    SineOsc sine = SineOsc();
    SineSawOsc sinesaw = SineSawOsc();
    
    
	void process(const ProcessArgs& args) override {
        float new_freq = params[FREQ_PARAM].getValue();
        if(_phase_inc == 0 || (_freq != new_freq)) {
            _freq = new_freq;
            _phase_inc = _twoPI * _freq / args.sampleRate;
        }
        
        // Sine Wave
//        float sin_out = sin(_phase); // this is also used for the square wave

        sinesaw.setPhaseParams(new_freq, args.sampleRate);
        float sin_out = sinesaw.getValue(VOICE_SINE);
        
        if(outputs[SIN_OUTPUT].isConnected()) {
            outputs[SIN_OUTPUT].setVoltage(5.f*sin_out);
        }

        
        // Triangle Wave
        if(outputs[TRI_OUTPUT].isConnected()) {
            float tri_out = _phase * 2.f / _PI;
            
            if(_phase < _PI/2.f) {
                //tri_out = 0.f + tri_out;
            } else if(_phase < _3halfPI) {
                tri_out = 2.f - tri_out;
            } else {
                tri_out = -4.f + tri_out;
            }
            
            outputs[TRI_OUTPUT].setVoltage(5.f*tri_out);
            
        }
               
        // Saw Wave
        if(outputs[RAMP_OUTPUT].isConnected()) {
//            outputs[RAMP_OUTPUT].setVoltage(5.f*(_phase/_PI - 1.f));
            outputs[RAMP_OUTPUT].setVoltage(5.f * sinesaw.getValue(VOICE_SAW));
        }
        sinesaw.next();
        
        // Square Wave
        if(outputs[SQR_OUTPUT].isConnected()) {
            float sqr_out = (sin_out > 0.f) ? 1.f : -1.f;
            outputs[SQR_OUTPUT].setVoltage(5.f*sqr_out);
        }
        
        // WHA th ...?
        if(outputs[WHA_OUTPUT].isConnected()) {
            float tri_out = _phase * 2.f / _PI;
            
            if(_phase < _PI/2.f) {
                tri_out = sin_out;
            } else if(_phase < _PI) {
                tri_out = (-sin_out*0.5f) + 0.2f;
            } else if(_phase < _3halfPI) {
                tri_out = (-sin_out*0.5f) - 0.2f;
            } else {
                tri_out = sin_out;
            }
    
            outputs[WHA_OUTPUT].setVoltage(5*tri_out);
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

		
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.f, 104.f)), module, Osc1::SIN_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.f, 104.f)), module, Osc1::RAMP_OUTPUT));
		
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.f, 111.5f)), module, Osc1::WHA_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.f, 119.f)), module, Osc1::TRI_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.f, 119.f)), module, Osc1::SQR_OUTPUT));
    }
};


Model* modelOsc1 = createModel<Osc1, Osc1Widget>("Osc1");
