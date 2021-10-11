/*
 * This is a wave-table oscillator. It is based on readings by Pirkle, Williams, & Roads
 *
 */

#include "plugin.hpp"
#include "lib/DirectOsc.hpp"
#include "lib/WavOsc.hpp"

struct Waves : Module {
	enum ParamIds {
		FREQ_PARAM,
		WAVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		// THRESH_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_WT_OUTPUT,
		AUDIO_FB_OUTPUT,
		AUDIO_GS_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	WavOscBase oscWT = WavOscBase(APP->engine->getSampleRate());
	FBSineOsc oscFB = FBSineOsc(APP->engine->getSampleRate());
	GordonSmithOsc oscGS = GordonSmithOsc(APP->engine->getSampleRate());

	Waves() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_PARAM, 0.f, 1000.f, 440.f, "Frequency (Hz)");
		// configParam(WAVE_PARAM, 0.f, 1.f, 0.f, "Choose the waveform");
		oscWT.loadWaveTable("/Users/daniel/projects/waveforms/sine_256.wav");
	}

	int count = 0;
	float output_gain = 5.f;
	void process(const ProcessArgs& args) override {
		float freq = params[FREQ_PARAM].getValue();

		oscWT.freq(freq);
		oscFB.freq(freq);
		oscGS.freq(freq);

		float outputWT = output_gain * oscWT.nextSample();
		float outputFB = output_gain * oscFB.nextSample();
		float outputGS = output_gain * oscGS.nextSample();

		outputs[AUDIO_WT_OUTPUT].setVoltage(outputWT);
		outputs[AUDIO_FB_OUTPUT].setVoltage(outputFB);
		outputs[AUDIO_GS_OUTPUT].setVoltage(outputGS);

            // std::cout << "output: " << output << std::endl;
        // dump out the state of things
        int log = false;
        // if(log && count++ % 20000 == 0) {
        if(log && count++ < 100) {
        // if(outputGS < 10) {
            std::cout << "outputGS: " << outputGS << std::endl;
				// ", output_lo: " << inputLo << 
				// ", output_mid: " << inputMid << 
				// ", output_hi: " << inputHi << std::endl;
        }
	}
};


struct WavesWidget : ModuleWidget {
	WavesWidget(Waves* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Waves.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float middle = 30;

		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(middle, 40)), module, Waves::FREQ_PARAM));
		// addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.398, 35.045)), module, Waves::WAVE_PARAM));

		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.861, 99.382)), module, Waves::THRESH_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(middle-10, 100)), module, Waves::AUDIO_WT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(middle, 100)), module, Waves::AUDIO_FB_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(middle+10, 100)), module, Waves::AUDIO_GS_OUTPUT));
	}
};


Model* modelWaves = createModel<Waves, WavesWidget>("Waves");