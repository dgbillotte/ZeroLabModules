#include "plugin.hpp"
#include "lib/Filter.hpp"

struct SpectralMix : Module {
	enum ParamIds {
		FREQ_LO_PARAM,
		FREQ_MID_PARAM,
		FREQ_HI_PARAM,
		SR_MIX_LO_PARAM,
		SR_MIX_MID_PARAM,
		SR_MIX_HI_PARAM,
		MIX_LO_PARAM,
		MIX_MID_PARAM,
		MIX_HI_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		RET_LO_INPUT,
		RET_MID_INPUT,
		RET_HI_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SEND_LO_OUTPUT,
		SEND_MID_OUTPUT,
		SEND_HI_OUTPUT,
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	/* What do we need:
	- HPF, BPF, LPF
	*/
	
	TwoPoleLPF _lpfLo1 = TwoPoleLPF(100, APP->engine->getSampleRate(), 1.f);
	TwoPoleLPF _lpfLo2 = TwoPoleLPF(100, APP->engine->getSampleRate(), 1.f);

	TwoPoleLPF _lpfMid1 = TwoPoleLPF(1000, APP->engine->getSampleRate(), 1.f);
	// TwoPoleLPF _lpfMid2 = TwoPoleLPF(1000, APP->engine->getSampleRate(), 1.f);
	TwoPoleHPF _hpfMid1 = TwoPoleHPF(1000, APP->engine->getSampleRate(), 1.f);
	// TwoPoleHPF _hpfMid2 = TwoPoleHPF(1000, APP->engine->getSampleRate(), 1.f);

	TwoPoleHPF _hpfHi1 = TwoPoleHPF(4000, APP->engine->getSampleRate(), 1.f);
	TwoPoleHPF _hpfHi2 = TwoPoleHPF(4000, APP->engine->getSampleRate(), 1.f);


	SpectralMix() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_LO_PARAM, 0.f, 1000.f, 100.f, "Freq: LO");
		configParam(FREQ_MID_PARAM, 100.f, 5000.f, 1000.f, "Freq: MID");
		configParam(FREQ_HI_PARAM, 1000.f, 10000.f, 4000.f, "Freq: HI");
		configParam(SR_MIX_LO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SR_MIX_MID_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SR_MIX_HI_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_LO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_MID_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_HI_PARAM, 0.f, 1.f, 0.f, "");


	}

	int count = 0;

	void process(const ProcessArgs& args) override {
		float freqLo = params[FREQ_LO_PARAM].getValue();
		float freqMid = params[FREQ_MID_PARAM].getValue();
		float freqHi = params[FREQ_HI_PARAM].getValue();


		// set filter params for lo band
		_lpfLo1.freq(freqLo);
		_lpfLo2.freq(freqLo);

		// set filter params for mid band
		_lpfMid1.freq(freqMid);
		_hpfMid1.freq(freqMid);
		// _lpfMid2.freq(freqMid);
		// _hpfMid2.freq(freqMid);

		// set filter params for hi band
		_hpfHi1.freq(freqHi);
		_hpfHi2.freq(freqHi);

		// get audio input and split out the 3 bands
		float input = inputs[AUDIO_INPUT].getVoltage();

		float inputLo = _lpfLo1.process(_lpfLo2.process(input));
		float inputMid = _lpfMid1.process(_hpfMid1.process(input));
		float inputHi = _hpfHi1.process(_hpfHi2.process(input));
		// float midInput = _lpfMid1.process(_lpfMid2.process(input));
		// midInput = _hpfMid1.process(_hpfMid2.process(midInput));

		// send each band out its send output
		outputs[SEND_LO_OUTPUT].setVoltage(inputLo);
		outputs[SEND_MID_OUTPUT].setVoltage(inputMid);
		outputs[SEND_HI_OUTPUT].setVoltage(inputHi);

		// read the returns and sr-mix params
		float returnLo = inputs[RET_LO_INPUT].getVoltage();
		float returnMid = inputs[RET_MID_INPUT].getVoltage();
		float returnHi = inputs[RET_HI_INPUT].getVoltage();
		
		float srMixLo = params[SR_MIX_LO_PARAM].getValue();
		float srMixMid = params[SR_MIX_MID_PARAM].getValue();
		float srMixHi = params[SR_MIX_HI_PARAM].getValue();
		
		float outLo = srMixLo*returnLo + (1-srMixLo)*inputLo;
		float outMid = srMixMid*returnMid + (1-srMixMid)*inputMid;
		float outHi = srMixHi*returnHi + (1-srMixHi)*inputHi;

		// read the mix params and compute the output
		float mixLo = params[MIX_LO_PARAM].getValue();
		float mixMid = params[MIX_MID_PARAM].getValue();
		float mixHi = params[MIX_HI_PARAM].getValue();

		float output = mixLo*outLo + mixMid*outMid + mixHi*outHi;
		outputs[AUDIO_OUTPUT].setVoltage(output);

        // dump out the state of things
        int log = true;
        if(!log && count++ % 20000 == 0) {
            std::cout << "input: " << input <<
				", output_lo: " << inputLo << 
				", output_mid: " << inputMid << 
				", output_hi: " << inputHi << std::endl;
        }
	}
};


struct SpectralMixWidget : ModuleWidget {
	SpectralMixWidget(SpectralMix* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SpectralMix.svg")));

		float width = 50.8f;
		float col1 = width/6;
		float col2 = width/2;
		float col3 = width - col1;

		float midX = 10.5f;
		float leftX = 5.f;
		float rightX = 16.f;

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Spectral Splitting
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 10.f)), module, SpectralMix::FREQ_LO_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 10.f)), module, SpectralMix::FREQ_MID_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col3, 10.f)), module, SpectralMix::FREQ_HI_PARAM));

		// Send / Return
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col1, 30)), module, SpectralMix::SEND_LO_OUTPUT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col1, 40)), module, SpectralMix::SR_MIX_LO_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col1, 50)), module, SpectralMix::RET_LO_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col2, 30)), module, SpectralMix::SEND_MID_OUTPUT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col2, 40)), module, SpectralMix::SR_MIX_MID_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, 50)), module, SpectralMix::RET_MID_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col3, 30)), module, SpectralMix::SEND_HI_OUTPUT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, 40)), module, SpectralMix::SR_MIX_HI_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col3, 50)), module, SpectralMix::RET_HI_INPUT));

		// Final Mix
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 90.f)), module, SpectralMix::MIX_LO_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 90.f)), module, SpectralMix::MIX_MID_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col3, 90.f)), module, SpectralMix::MIX_HI_PARAM));

		// Input / Output
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col1*2, 115)), module, SpectralMix::AUDIO_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col1*4, 115)), module, SpectralMix::AUDIO_OUTPUT));
	}
};


Model* modelSpectralMix = createModel<SpectralMix, SpectralMixWidget>("SpectralMix");