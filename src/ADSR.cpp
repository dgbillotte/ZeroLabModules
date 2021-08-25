#include "plugin.hpp"


using namespace simd;


const float MIN_TIME = 1e-3f;
const float MAX_TIME = 10.f;
const float LAMBDA_BASE = MAX_TIME / MIN_TIME;


struct ADSR : Module {
	enum ParamIds {
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		GATE_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENVELOPE_OUTPUT,
		ATTACK_GATE_OUTPUT,
		DECAY_GATE_OUTPUT,
		SUSTAIN_GATE_OUTPUT,
		RELEASE_GATE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ATTACK_LIGHT,
		DECAY_LIGHT,
		SUSTAIN_LIGHT,
		RELEASE_LIGHT,
		NUM_LIGHTS
	};

	float_4 attacking[4] = {float_4::zero()};
	float_4 env[4] = {0.f};
	dsp::TSchmittTrigger<float_4> trigger[4];
	dsp::ClockDivider cvDivider;
	float_4 attackLambda[4] = {0.f};
	float_4 decayLambda[4] = {0.f};
	float_4 releaseLambda[4] = {0.f};
	float_4 sustain[4] = {0.f};
	dsp::ClockDivider lightDivider;

	ADSR() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.5f, "Attack", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(DECAY_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 0.5f, "Sustain", "%", 0, 100);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.5f, "Release", " ms", LAMBDA_BASE, MIN_TIME * 1000);

		cvDivider.setDivision(16);
		lightDivider.setDivision(128);
	}

	void process(const ProcessArgs& args) override {
		// 0.16-0.19 us serial
		// 0.23 us serial with all lambdas computed
		// 0.15-0.18 us serial with all lambdas computed with SSE

		int channels = inputs[GATE_INPUT].getChannels();

		// Compute lambdas
		if (cvDivider.process()) {
			float attackParam = params[ATTACK_PARAM].getValue();
			float decayParam = params[DECAY_PARAM].getValue();
			float sustainParam = params[SUSTAIN_PARAM].getValue();
			float releaseParam = params[RELEASE_PARAM].getValue();

			for (int c = 0; c < channels; c += 4) {
				// CV
				float_4 attack = attackParam + inputs[ATTACK_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;
				float_4 decay = decayParam + inputs[DECAY_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;
				float_4 sustain = sustainParam + inputs[SUSTAIN_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;
				float_4 release = releaseParam + inputs[RELEASE_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f;

				attack = simd::clamp(attack, 0.f, 1.f);
				decay = simd::clamp(decay, 0.f, 1.f);
				sustain = simd::clamp(sustain, 0.f, 1.f);
				release = simd::clamp(release, 0.f, 1.f);

				attackLambda[c / 4] = simd::pow(LAMBDA_BASE, -attack) / MIN_TIME;
				decayLambda[c / 4] = simd::pow(LAMBDA_BASE, -decay) / MIN_TIME;
				releaseLambda[c / 4] = simd::pow(LAMBDA_BASE, -release) / MIN_TIME;
				this->sustain[c / 4] = sustain;
			}
		}

		float_4 gate[4];

		for (int c = 0; c < channels; c += 4) {
			// Gate
			gate[c / 4] = inputs[GATE_INPUT].getVoltageSimd<float_4>(c) >= 1.f;

			// Retrigger
			float_4 triggered = trigger[c / 4].process(inputs[TRIG_INPUT].getPolyVoltageSimd<float_4>(c));
			attacking[c / 4] = simd::ifelse(triggered, float_4::mask(), attacking[c / 4]);

			// Get target and lambda for exponential decay
			const float attackTarget = 1.2f;
			float_4 target = simd::ifelse(gate[c / 4], simd::ifelse(attacking[c / 4], attackTarget, sustain[c / 4]), 0.f);
			float_4 lambda = simd::ifelse(gate[c / 4], simd::ifelse(attacking[c / 4], attackLambda[c / 4], decayLambda[c / 4]), releaseLambda[c / 4]);

			// Adjust env
			env[c / 4] += (target - env[c / 4]) * lambda * args.sampleTime;

			// Turn off attacking state if envelope is HIGH
			attacking[c / 4] = simd::ifelse(env[c / 4] >= 1.f, float_4::zero(), attacking[c / 4]);

			// Turn on attacking state if gate is LOW
			attacking[c / 4] = simd::ifelse(gate[c / 4], attacking[c / 4], float_4::mask());

			// Set output
			outputs[ENVELOPE_OUTPUT].setVoltageSimd(10.f * env[c / 4], c);
		}

		outputs[ENVELOPE_OUTPUT].setChannels(channels);

		// Lights
		if (lightDivider.process()) {
			lights[ATTACK_LIGHT].setBrightness(0);
			lights[DECAY_LIGHT].setBrightness(0);
			lights[SUSTAIN_LIGHT].setBrightness(0);
			lights[RELEASE_LIGHT].setBrightness(0);

			for (int c = 0; c < channels; c += 4) {
				const float epsilon = 0.01f;
				float_4 sustaining = (sustain[c / 4] <= env[c / 4]) & (env[c / 4] < sustain[c / 4] + epsilon);
				float_4 resting = (env[c / 4] < epsilon);

				if (simd::movemask(gate[c / 4] & attacking[c / 4])) {
					lights[ATTACK_LIGHT].setBrightness(1);
					outputs[ATTACK_GATE_OUTPUT].setVoltage(10);
				} else {
					outputs[ATTACK_GATE_OUTPUT].setVoltage(0);
				}
				if (simd::movemask(gate[c / 4] & ~attacking[c / 4] & ~sustaining)) {
					lights[DECAY_LIGHT].setBrightness(1);
					outputs[DECAY_GATE_OUTPUT].setVoltage(10);
				} else {
					outputs[DECAY_GATE_OUTPUT].setVoltage(0);

				}
				if (simd::movemask(gate[c / 4] & ~attacking[c / 4] & sustaining)) {
					lights[SUSTAIN_LIGHT].setBrightness(1);
					outputs[SUSTAIN_GATE_OUTPUT].setVoltage(10);
				} else {
					outputs[SUSTAIN_GATE_OUTPUT].setVoltage(0);

				}
				if (simd::movemask(~gate[c / 4] & ~resting)) {
					lights[RELEASE_LIGHT].setBrightness(1);
					outputs[RELEASE_GATE_OUTPUT].setVoltage(10);
				} else {
					outputs[RELEASE_GATE_OUTPUT].setVoltage(0);

				}

			}
		}
	}
};


struct ADSRWidget : ModuleWidget {
	ADSRWidget(ADSR* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ADSR-gates.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		int c1 = 9; // orig val is 9
		int c2 = 42; // orig val is 62
		int c3 = 80; // this didn't exist
		int c4 = 94; // orig val is 94

		addParam(createParam<RoundLargeBlackKnob>(Vec(c2, 57), module, ADSR::ATTACK_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(c2, 124), module, ADSR::DECAY_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(c2, 191), module, ADSR::SUSTAIN_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(c2, 257), module, ADSR::RELEASE_PARAM));

		addInput(createInput<PJ301MPort>(Vec(c1, 63), module, ADSR::ATTACK_INPUT));
		addInput(createInput<PJ301MPort>(Vec(c1, 129), module, ADSR::DECAY_INPUT));
		addInput(createInput<PJ301MPort>(Vec(c1, 196), module, ADSR::SUSTAIN_INPUT));
		addInput(createInput<PJ301MPort>(Vec(c1, 263), module, ADSR::RELEASE_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(c3, 63), module, ADSR::ATTACK_GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(c3, 129), module, ADSR::DECAY_GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(c3, 196), module, ADSR::SUSTAIN_GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(c3, 263), module, ADSR::RELEASE_GATE_OUTPUT));

		addInput(createInput<PJ301MPort>(Vec(9, 320), module, ADSR::GATE_INPUT));
		addInput(createInput<PJ301MPort>(Vec(48, 320), module, ADSR::TRIG_INPUT));
		addOutput(createOutput<PJ301MPort>(Vec(87, 320), module, ADSR::ENVELOPE_OUTPUT));

		addChild(createLight<SmallLight<RedLight>>(Vec(c4, 41), module, ADSR::ATTACK_LIGHT));
		addChild(createLight<SmallLight<RedLight>>(Vec(c4, 109), module, ADSR::DECAY_LIGHT));
		addChild(createLight<SmallLight<RedLight>>(Vec(c4, 175), module, ADSR::SUSTAIN_LIGHT));
		addChild(createLight<SmallLight<RedLight>>(Vec(c4, 242), module, ADSR::RELEASE_LIGHT));
	}
};


Model* modelADSR = createModel<ADSR, ADSRWidget>("ADSR");
