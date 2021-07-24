#include "plugin.hpp"

#define BIQUAD(a0, x0, a1, x1, a2, x2, b1, y1, b2, y2) (a0*x0 + a1*x1 + a2*x2 - b1*y1 - b2*y2)

struct SecondOrderBiQuad : Module {
	enum ParamIds {
		FREQKNOB_PARAM,
		Q_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		HPF_OUTPUT,
		LPF_OUTPUT,
		BPF_OUTPUT,
		BSF_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    float x_0=0, x_1=0, x_2=0,
        y_hpf_0=0, y_hpf_1=0, y_hpf_2=0,
        y_lpf_0=0, y_lpf_1=0, y_lpf_2=0,
        y_bpf_0=0, y_bpf_1=0, y_bpf_2=0,
        y_bsf_0=0, y_bsf_1=0, y_bsf_2=0;
    
    float TWO_PI = 2*M_PI;
    int sample_rate = 41000;
    
	SecondOrderBiQuad() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 0.f, 10000.f, 0.f, "");
		configParam(Q_PARAM, 0.f, 11.f, 0.f, "");
	}

    
	void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        float q = params[Q_PARAM].getValue();
        x_0 = inputs[AUDIO_INPUT].getVoltage();
        
        // shared coefficient calculation
        float theta = TWO_PI*freq/sample_rate;
        
        // shared LPF/HPF coefficients
        float d = 1/q;
        float beta = 0.5*(1 - d*sin(theta)/2) / (1 + d*sin(theta)/2);
        float gamma = (0.5 + beta)*cos(theta);
        
        // LPF coefficients
        float a_1 = 0.5 + beta - gamma;
        float a_0 = a_1/2;
        float a_2 = a_0;
        float b_1 = -2*gamma;
        float b_2 = 2*beta;
        
        // compute LPF output
        y_lpf_0 = a_0*x_0 + a_1*x_1 + a_2*x_2 - b_1*y_lpf_1 - b_2*y_lpf_2;
        
        // HPF coefficients
        a_1 = 0.5 + beta + gamma;
        a_0 = a_1/2;
        a_1 = -a_1;
        a_2 = a_0;
        // b_1 & b_2 are the same as for LPF
        
        // compute HPF output
        y_hpf_0 = a_0*x_0 + a_1*x_1 + a_2*x_2 - b_1*y_hpf_1 - b_2*y_hpf_2;
        
        // shared BPF/BSF coefficients
//        float tmp1 = theta/(2*q) >= M_PI/2 ? nextafter(M_PI/2, 0.0f) : theta/(2*q);
        
        float tmp1 = clamp(theta/(2*q), 0.0f, nextafter(M_PI/2, 0.0f));
        
        
        float tmp = tan(tmp1);
        beta = 0.5*(1 - tmp)/(1 + tmp);
        gamma = (0.5 + beta)*cos(theta);
        
        // BPF coefficients
        a_0 = 0.5 - beta;
        a_1 = 0;
        a_2 = -a_0;
        // b_1 & b_2 are the same as for LPF/HPF
        
        // compute the BPF output
        y_bpf_0 = a_0*x_0 + a_1*x_1 + a_2*x_2 - b_1*y_bpf_1 - b_2*y_bpf_2;
        
        // BSF coefficients
        a_0 = 0.5 + beta;
        a_1 = -2 * gamma;
        a_2 = 0.5 + beta;
        // b_1 & b_2 are the same as for LPF/HPF
        
        // compute the BSF output
        y_bsf_0 = a_0*x_0 + a_1*x_1 + a_2*x_2 - b_1*y_bsf_1 - b_2*y_bsf_2;
        
        
        // write the outputs
        outputs[LPF_OUTPUT].setVoltage(y_lpf_0);
        outputs[HPF_OUTPUT].setVoltage(y_hpf_0);
        outputs[BPF_OUTPUT].setVoltage(y_bpf_0);
        outputs[BSF_OUTPUT].setVoltage(y_bsf_0);
        
        // set the delays
        x_2 = x_1;
        x_1 = x_0;
        y_lpf_2 = y_lpf_1;
        y_lpf_1 = y_lpf_0;
        y_hpf_2 = y_hpf_1;
        y_hpf_1 = y_hpf_0;
        y_bpf_2 = y_bpf_1;
        y_bpf_1 = y_bpf_0;
        y_bsf_2 = y_bsf_1;
        y_bsf_1 = y_bsf_0;
	}
};


struct SecondOrderBiQuadWidget : ModuleWidget {
	SecondOrderBiQuadWidget(SecondOrderBiQuad* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SecondOrderBiQuad.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.262, 14.504)), module, SecondOrderBiQuad::FREQKNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.262, 31.321)), module, SecondOrderBiQuad::Q_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.262, 53.342)), module, SecondOrderBiQuad::AUDIO_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 70.496)), module, SecondOrderBiQuad::HPF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 84.766)), module, SecondOrderBiQuad::LPF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 99.035)), module, SecondOrderBiQuad::BPF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 113.304)), module, SecondOrderBiQuad::BSF_OUTPUT));
	}
};


Model* modelSecondOrderBiQuad = createModel<SecondOrderBiQuad, SecondOrderBiQuadWidget>("SecondOrderBiQuad");
