#include "plugin.hpp"
#include "lib/Filter.hpp"
#include "lib/Components.hpp"

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
		RET_LO_CV_INPUT,
		RET_MID_INPUT,
		RET_MID_CV_INPUT,
		RET_HI_INPUT,
		RET_HI_CV_INPUT,
		MIX_LO_CV_INPUT,
		MIX_MID_CV_INPUT,
		MIX_HI_CV_INPUT,
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
		configParam(FREQ_LO_PARAM, 20.f, 1000.f, 100.f, "Freq: LO");
		configParam(FREQ_MID_PARAM, 100.f, 5000.f, 1000.f, "Freq: MID");
		configParam(FREQ_HI_PARAM, 1000.f, 10000.f, 4000.f, "Freq: HI");
		configParam(SR_MIX_LO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SR_MIX_MID_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SR_MIX_HI_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_LO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_MID_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX_HI_PARAM, 0.f, 1.f, 0.f, "");
	}

	// handle changes to the sample rate
	void onSampleRateChange() override {
		int sampleRate = APP->engine->getSampleRate();
		_lpfLo1.sampleRate(sampleRate);
		_lpfLo2.sampleRate(sampleRate);
		_lpfMid1.sampleRate(sampleRate);
		_hpfMid1.sampleRate(sampleRate);
		_hpfHi1.sampleRate(sampleRate);
		_hpfHi2.sampleRate(sampleRate);
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
		float mixLo = params[MIX_LO_PARAM].getValue() + inputs[MIX_LO_CV_INPUT].getVoltage();;
		float mixMid = params[MIX_MID_PARAM].getValue() + inputs[MIX_MID_CV_INPUT].getVoltage();
		float mixHi = params[MIX_HI_PARAM].getValue() + inputs[MIX_HI_CV_INPUT].getVoltage();

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
	// module dimensions and handy guides
	float width = 50.8;
	float midX = width/2;
	float height = 128.5;
	// float _6th = width/6;
	// float _5_6th = width-_6th;
	// float _8th = width/8;
	// float _7_8th = width-_8th;
	// float gutter = 5.f;
	float col1 = width/6;
	float col2 = width/2;
	float col3 = width - col1;

	SpectralMixWidget(SpectralMix* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SpectralMix.svg")));

		// screws
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Spectral Splitting
		float rowY = 18.f; 
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, SpectralMix::FREQ_LO_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, SpectralMix::FREQ_MID_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, SpectralMix::FREQ_HI_PARAM));

		// Send / Return

		rowY += 15;
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col1, rowY)), module, SpectralMix::SEND_LO_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col2, rowY)), module, SpectralMix::SEND_MID_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, SpectralMix::SEND_HI_OUTPUT));

		rowY += 13;
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(col1, rowY)), module, SpectralMix::SR_MIX_LO_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(col2, rowY)), module, SpectralMix::SR_MIX_MID_PARAM));
		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(col3, rowY)), module, SpectralMix::SR_MIX_HI_PARAM));

		rowY += 13;
		addInput(createInputCentered<CVInputJack>(mm2px(Vec(col1, rowY)), module, SpectralMix::RET_LO_CV_INPUT));
		addInput(createInputCentered<CVInputJack>(mm2px(Vec(col2, rowY)), module, SpectralMix::RET_MID_CV_INPUT));
		addInput(createInputCentered<CVInputJack>(mm2px(Vec(col3, rowY)), module, SpectralMix::RET_HI_CV_INPUT));

		rowY += 13;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, SpectralMix::RET_LO_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col2, rowY)), module, SpectralMix::RET_MID_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col3, rowY)), module, SpectralMix::RET_HI_INPUT));


		// Final Mix
		rowY = 87;
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col1, rowY)), module, SpectralMix::MIX_LO_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col2, rowY)), module, SpectralMix::MIX_MID_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col3, rowY)), module, SpectralMix::MIX_HI_PARAM));

		rowY = 100;
		addInput(createInputCentered<CVInputJack>(mm2px(Vec(col1, rowY)), module, SpectralMix::MIX_LO_CV_INPUT));
		addInput(createInputCentered<CVInputJack>(mm2px(Vec(col2, rowY)), module, SpectralMix::MIX_MID_CV_INPUT));
		addInput(createInputCentered<CVInputJack>(mm2px(Vec(col3, rowY)), module, SpectralMix::MIX_HI_CV_INPUT));

		// Input / Output
		rowY = 113;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(col1, rowY)), module, SpectralMix::AUDIO_INPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(col3, rowY)), module, SpectralMix::AUDIO_OUTPUT));
	}
};


Model* modelSpectralMix = createModel<SpectralMix, SpectralMixWidget>("SpectralMix");