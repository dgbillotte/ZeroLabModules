#include "plugin.hpp"

#include "lib/Components.hpp"
#include "lib/DelayBuffer.hpp"


struct Loopy : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		RECORD_GATE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const int MAX_DELAY;
	DelayBuffer<float> _loopBuffer;

	// dsp::SchmittTrigger _pluckTrig;
	// dsp::SchmittTrigger _refretTrig;

	Loopy() : MAX_DELAY(100000), _loopBuffer(MAX_DELAY)	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void onSampleRateChange() override;

	int downsampleCount = 0;
	int downsampleRate = 16;
	void process(const ProcessArgs& args) override;

};

void Loopy::onSampleRateChange() {
	int sampleRate = APP->engine->getSampleRate();
	// _kpString.sampleRate(sampleRate);
	// _resonator.sampleRate(sampleRate);
}


bool _recording = false;
size_t _loopLength = 0;
size_t _playIndex = 0;

void Loopy::process(const ProcessArgs& args) {
	// only process non-audio params every downsampleRate samples
	// if(downsampleCount++ == downsampleRate) {
	// 	downsampleCount = 0;
	// }

	float audioIn = inputs[AUDIO_INPUT].getVoltage();
	float recordGate = inputs[RECORD_GATE_INPUT].getVoltage();
	if(recordGate > 3.f) { // I use +3V so that a +/-5V binary signal will work as well as +0/10V
		if(! _recording) {
			// start recording
			_recording = true;
			_loopBuffer.resetHead();
			_loopLength = 0;
		}
		// continue recording
		_loopBuffer.push(audioIn);
		_loopLength++;
		
	} else {
		if(_recording) {
			// stop recording
			_recording = false;
			_playIndex = 0;
		}

		// continue not recording
	}

	// with this setup, while recording it will continue to play based on the previous sample
	// when recording finishes, loop playback will reset to the begining
	if(_loopLength > 0) {
		float sampleOut = _loopBuffer.aRead(_playIndex);
		if(++_playIndex == _loopLength) {
			_playIndex = 0;
		}

		outputs[AUDIO_OUTPUT].setVoltage(sampleOut);
	}
}



//------------------------------------------------------------
struct LoopyWidget : ModuleWidget {

	float width = 50.8;
	float midX = width/2;
	float height = 128.5;
    float midY = height/2;
	float _8th = width/8;
	float _7_8th = width-_8th;
    float gutter = 5.f;

	// for 3 columns
	float col1 = width/6;
	float col2 = width/2;
	float col3 = width - col1;	


	LoopyWidget(Loopy* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Loopy.svg")));

		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float rowInc = 18;
		float rowY = 18;
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Loopy::PLUCK_FREQ_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Loopy::DECAY_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Loopy::STRETCH_PARAM));

		// rowY += rowInc;
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Loopy::ATTACK_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Loopy::PICK_POS_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Loopy::IMPUSE_LPF_PARAM));

		// rowY += rowInc;
		// addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col1, rowY)), module, Loopy::BODY_SIZE_PARAM));
		// addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col2, rowY)), module, Loopy::RES_Q_PARAM));
		// addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col3, rowY)), module, Loopy::RES_MIX_PARAM));

		// rowY += rowInc+10;
		// addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(Vec(col2, rowY)), module, Loopy::IMPULSE_TYPE_PARAM));
		

		// top row of jacks
		rowY = 87.f;

		// middle row of jacks
		rowY = 100.f;
		// addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, Loopy::PLUCK_VOCT_INPUT));
		// addParam(createParamCentered<NKK>(mm2px(Vec(col2, rowY)), module, Loopy::PICK_POS_ON_PARAM));
		// addParam(createParamCentered<NKK>(mm2px(Vec(col3, rowY)), module, Loopy::IMPULSE_LPF_ON_PARAM));

		// bottom row of jacks
		rowY = 113.f;
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(_8th, rowY)), module, Loopy::AUDIO_INPUT));
		addInput(createInputCentered<AudioInputJack>(mm2px(Vec(3*_8th, rowY)), module, Loopy::RECORD_GATE_INPUT));
		// addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(5*_8th, rowY)), module, Loopy::DRY_OUTPUT));
		addOutput(createOutputCentered<AudioOutputJack>(mm2px(Vec(_7_8th, rowY)), module, Loopy::AUDIO_OUTPUT));
	}
};

Model* modelLoopy = createModel<Loopy, LoopyWidget>("Loopy");