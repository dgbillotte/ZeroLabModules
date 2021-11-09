#include "plugin.hpp"

#include "lib/Components.hpp"
#include "lib/DelayBuffer.hpp"

/*
 * VCV Things to figure out:
 * - how to set a knob
 * - better understanding of components: how to build them
 */
struct Loopy : Module {
	enum ParamIds {
		PHASE_INC_PARAM,
		LOOP_START_PARAM,
		LOOP_LENGTH_PARAM,
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

	bool _recording = false;
	size_t _loopLength = 0;
	float _playFIdx = 0.f;
	int _startIndex = 0;
	bool _startIndexChanged = false;

	// parameters
	float _phaseIncr = 1.f;
	int _loopStartOffset = 0.f;
	float _loopPlayPct = 1.f;
	int _loopPlayLength = 0;
	


	// dsp::SchmittTrigger _pluckTrig;
	// dsp::SchmittTrigger _refretTrig;

	Loopy() : MAX_DELAY(4410), _loopBuffer(MAX_DELAY)	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PHASE_INC_PARAM, -1.f, 1.f, 0.f, "Phase Increment Offset");
		configParam(LOOP_START_PARAM, 0.f, 1000.f, 0.f, "Loop Start Offset in samples");
		// configParam(LOOP_LENGTH_PARAM, -1.f, 1.f, 1.f, "Loop Length as a %");
		configParam(LOOP_LENGTH_PARAM, 0.f, 1.f, 1.f, "Loop Length as a %");
	}

	void onSampleRateChange() override;

	int downsampleCount = 0;
	int downsampleRate = 16;
	void process(const ProcessArgs& args) override;
	float _processSample(float input, bool record);
	float _nextSample();

};

void Loopy::onSampleRateChange() {
	int sampleRate = APP->engine->getSampleRate();
	// _kpString.sampleRate(sampleRate);
	// _resonator.sampleRate(sampleRate);
}


void Loopy::process(const ProcessArgs& args) {
	// only process non-audio params every downsampleRate samples
	if(downsampleCount++ == downsampleRate) {
		downsampleCount = 0;

		float phaseIncOffset = params[PHASE_INC_PARAM].getValue();
		_phaseIncr = 1.f + phaseIncOffset;
		
		float loopPlayPct = params[LOOP_LENGTH_PARAM].getValue();
		if(loopPlayPct != _loopPlayPct) {
			_loopPlayPct = loopPlayPct;
			_loopPlayLength = _loopLength * _loopPlayPct;

		}

		float startIndex = params[LOOP_START_PARAM].getValue();
		if(_startIndex != startIndex) {
			_startIndex = startIndex;
			_startIndexChanged = true;
		}
	}

	float audioIn = inputs[AUDIO_INPUT].getVoltage();
	bool recordGate = inputs[RECORD_GATE_INPUT].getVoltage() > 3.f; // +3V works for all signal ranges we see
	float audioOut = _processSample(audioIn, recordGate);
	outputs[AUDIO_OUTPUT].setVoltage(audioOut);
}

inline float Loopy::_processSample(float audioIn, bool record) {

	if(record) { 
		// start recording
		if(! _recording) {
			_recording = true;
			_loopBuffer.resetHead();
			_loopLength = 0;
		}

		// continue recording and play the current input
		_loopBuffer.push(audioIn);
		_loopLength++;

		// return current sample in as the sample out
		return audioIn;	
	}
	 
	// stop recording
	if(_recording) {
		_recording = false;
		if(_loopLength > MAX_DELAY) {
			_loopLength = MAX_DELAY;
		}
		_playFIdx = _startIndex;
		_loopPlayLength = _loopLength * _loopPlayPct;
	}

	// return next sample from the loop
	return _nextSample();
}

inline float Loopy::_nextSample() {
	if(_loopLength == 0) {
		return 0.f;
	}

	int x0 = (int)_playFIdx;
	float y0 = _loopBuffer.aRead(x0);
		
	// this could be a float, would it make an audible difference?
	int stop = _loopLength * _loopPlayPct;

	int x1 = (x0+1 < stop) ? x0+1 : 0;
	float y1 = _loopBuffer.aRead(x1);

	float sampleOut = y0 + (_playFIdx - x0) * (y1 - y0);

	_playFIdx += _phaseIncr;
	if(_playFIdx >= stop) {
		if(_startIndexChanged) {
			_playFIdx = _startIndex;
			_startIndexChanged = false;
		} else {
			_playFIdx -= stop;
		}
	}

	return sampleOut;
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
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Loopy::PHASE_INC_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Loopy::DECAY_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Loopy::STRETCH_PARAM));

		rowY += rowInc;
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col1, rowY)), module, Loopy::LOOP_START_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col2, rowY)), module, Loopy::LOOP_LENGTH_PARAM));
		// addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(col3, rowY)), module, Loopy::IMPUSE_LPF_PARAM));

		// rowY += rowInc;
		// addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col1, rowY)), module, Loopy::BODY_SIZE_PARAM));
		// addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col2, rowY)), module, Loopy::RES_Q_PARAM));
		// addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(col3, rowY)), module, Loopy::RES_MIX_PARAM));

		// rowY += rowInc+10;
		// addParam(createParamCentered<Davies1900hLargeRedKnob>(mm2px(Vec(col2, rowY)), module, Loopy::IMPULSE_TYPE_PARAM));
		
		rowY += rowInc;
		// addChild(createWidge98t<TextField>)
		

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