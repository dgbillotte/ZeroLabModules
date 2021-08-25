#include "plugin.hpp"
//#include "AudioLib.hpp"
#include "Osc.hpp"

struct Osc1 : Module {
	enum InputIds {
		VOCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIN_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		SINSUB1_OUTPUT,
        SQRSUB1_OUTPUT,
        SINSUB2_OUTPUT,
        SQRSUB2_OUTPUT,
		NUM_OUTPUTS
	};
    enum ParamIds {
        FREQ_PARAM,
        // RANGE_PARAM,
        // POLARITY_PARAM,
        NUM_PARAMS
    };
	enum LightIds {
		NUM_LIGHTS
	};
    
    // SineSawOsc sinesaw = SineSawOsc();
    // TriangleOsc triangle = TriangleOsc();
    // DIYQuadrant quad = DIYQuadrant();
    Phatty phatty = Phatty();
    
	Osc1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_PARAM, 0, 10000, 440, "Freq(Hz)");
		// configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "");
		// configParam(POLARITY_PARAM, 0.f, 1.f, 0.f, "");
        
        // quad.setAllQ([](float phase){ return sin(phase); },
        //              [](float phase){ return -1.f + sin(phase); },
        //              [](float phase){ return -sin(phase); },
        //              [](float phase){ return 1.f - sin(phase); });
	}
    
    // void onSampleRateChange() override {
    //     sinesaw.setSampleRate()
    // }

	void process(const ProcessArgs& args) override {
        float freq = params[FREQ_PARAM].getValue();
        
        // quad.setPhaseParams(freq, args.sampleRate);
        // sinesaw.setPhaseParams(freq, args.sampleRate);
        // triangle.setPhaseParams(freq, args.sampleRate);
        phatty.setPhaseParams(freq, args.sampleRate);
        
        outputs[SIN_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_SINE1));
        outputs[SAW_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_SAW1));
        outputs[SQR_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_SQR1));
        outputs[SINSUB1_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_WHA_SUB1));
        outputs[SQRSUB1_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_SQR_SUB1));
        outputs[SINSUB2_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_WHA_SUB2));
        outputs[SQRSUB2_OUTPUT].setVoltage(5 * phatty.getValue(VOICE_SQR_SUB2));
        
        phatty.next();

//         // Sine Wave
//         float sin_out = sinesaw.getValue(VOICE_SINE);
//         if(outputs[SIN_OUTPUT].isConnected()) {
//             outputs[SIN_OUTPUT].setVoltage(5.f*phatty.getValue(VOICE_SINE1));
//         }
        
//         // Triangle Wave
//         if(outputs[TRI_OUTPUT].isConnected()) {
//             float tri_out = triangle.getValue();
//             outputs[TRI_OUTPUT].setVoltage(5.f*tri_out);
//             outputs[TRI_OUTPUT].setVoltage(5.f*phatty.getValue(VOICE_SINE_SUB2));
//         }
               
//         // Saw Wave
//         if(outputs[RAMP_OUTPUT].isConnected()) {
// //            outputs[RAMP_OUTPUT].setVoltage(5.f*(_phase/_PI - 1.f));
//             outputs[RAMP_OUTPUT].setVoltage(5.f * sinesaw.getValue(VOICE_SAW));
//         }
        
//         // Square Wave
//         if(outputs[SQR_OUTPUT].isConnected()) {
//             // float sqr_out = (sin_out > 0.f) ? 1.f : -1.f;
//             // outputs[SQR_OUTPUT].setVoltage(5.f*sqr_out);
//             outputs[SQR_OUTPUT].setVoltage(5.f*phatty.getValue(VOICE_SQR1));
//         }
        
//         // WHA th ...?
//         if(outputs[WHA_OUTPUT].isConnected()) {
//             // float out = quad.getValue();
//             // outputs[WHA_OUTPUT].setVoltage(5*tri_out);
//             outputs[WHA_OUTPUT].setVoltage(5.f*phatty.getValue(VOICE_SINE_SUB1));
//         }

//         sinesaw.next();
//         phatty.next();
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

		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(9.62, 74.753)), module, Osc1::RANGE_PARAM));
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.033, 74.981)), module, Osc1::POLARITY_PARAM));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.967, 48.537)), module, Osc1::VOCT_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.f, 90.f)), module, Osc1::SIN_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.f, 90.f)), module, Osc1::SQR_OUTPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.f, 110.f)), module, Osc1::SAW_OUTPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.f, 105.f)), module, Osc1::SINSUB1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.f, 105.f)), module, Osc1::SQRSUB1_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.f, 120.f)), module, Osc1::SINSUB2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.f, 120.f)), module, Osc1::SQRSUB2_OUTPUT));
    }
};


Model* modelOsc1 = createModel<Osc1, Osc1Widget>("Osc1");
