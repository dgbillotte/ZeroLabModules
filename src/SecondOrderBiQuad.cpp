#include "plugin.hpp"
#include "lib/Filter.hpp"


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
    
    TwoPoleLPF* lpf = new TwoPoleLPF();
    TwoPoleHPF* hpf = new TwoPoleHPF();
    TwoPoleBPF* bpf = new TwoPoleBPF();
    TwoPoleBSF* bsf = new TwoPoleBSF();
    
	SecondOrderBiQuad() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQKNOB_PARAM, 0.f, 10000.f, 0.f, "");
		configParam(Q_PARAM, 0.f, 11.f, 0.f, "");
	}

    void process(const ProcessArgs& args) override {
        float freq = params[FREQKNOB_PARAM].getValue();
        float q = params[Q_PARAM].getValue();
        float input = inputs[AUDIO_INPUT].getVoltage();
        
        lpf->freq(freq);
        lpf->q(q);
        hpf->freq(freq);
        hpf->q(q);
        bpf->freq(freq);
        bpf->q(q);
        bsf->freq(freq);
        bsf->q(q);
        
        outputs[LPF_OUTPUT].setVoltage(lpf->process(input));
        outputs[HPF_OUTPUT].setVoltage(hpf->process(input));
        outputs[BPF_OUTPUT].setVoltage(bpf->process(input));
        outputs[BSF_OUTPUT].setVoltage(bsf->process(input));
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

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 70.496)), module, SecondOrderBiQuad::LPF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 84.766)), module, SecondOrderBiQuad::HPF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 99.035)), module, SecondOrderBiQuad::BPF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.262, 113.304)), module, SecondOrderBiQuad::BSF_OUTPUT));
	}
};


Model* modelSecondOrderBiQuad = createModel<SecondOrderBiQuad, SecondOrderBiQuadWidget>("SecondOrderBiQuad");
