#include "plugin.hpp"
#include "CBuffer.hpp"

/*
	Notes: I'm pretty excited about the way this turned out on the first pass. Some ideas:
	- feedback: could have just level 1 feedback or could try to have the different outputs
	  exhibit different feedback based on their position or a knob, have to think about this...

*/


struct WeirdDelay : Module {
        enum ParamIds {
                DELAY1_KNOB,
                DELAY1_CV_KNOB,
                DELAY2_KNOB,
                DELAY2_CV_KNOB,
                DELAY3_KNOB,
                DELAY3_CV_KNOB,
                DELAY4_KNOB,
                DELAY4_CV_KNOB,
                NUM_PARAMS
        };
        enum InputIds {
                IN1_INPUT,
                DELAY1_CV_INPUT,
                DELAY2_CV_INPUT,
                DELAY3_CV_INPUT,
                DELAY4_CV_INPUT,
                // IN3_INPUT,
                // IN4_INPUT,
                NUM_INPUTS
        };
        enum OutputIds {
                DELAY1_OUTPUT,
                DELAY2_OUTPUT,
                DELAY3_OUTPUT,
                DELAY4_OUTPUT,
                NUM_OUTPUTS
        };
        enum LightIds {
                // OUT_POS_LIGHT,
                // OUT_NEG_LIGHT,
                NUM_LIGHTS
        };

        CBuffer* delay;

        WeirdDelay() {
                config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
                configParam(DELAY1_KNOB, 0.0, 1.0, 0.0, "Main Delay", "%", 0, 100);
                configParam(DELAY1_CV_KNOB, -1.0, 1.0, 0.0, "Main Delay CV Attn", "%", 0, 100);
                configParam(DELAY2_KNOB, -2.0, 2.0, 0.0, "Delay 2 Ratio", "%", 0, 100);
                configParam(DELAY2_CV_KNOB, -1.0, 1.0, 0.0, "Delay 2 CV Attn", "%", 0, 100);
                configParam(DELAY3_KNOB, -2.0, 2.0, 0.0, "Delay 3 Ratio", "%", 0, 100);
                configParam(DELAY3_CV_KNOB, -1.0, 1.0, 0.0, "Delay 3 CV Attn", "%", 0, 100);
                configParam(DELAY4_KNOB, -2.0, 2.0, 0.0, "Delay 4 Ratio", "%", 0, 100);
                configParam(DELAY4_CV_KNOB, -1.0, 1.0, 0.0, "Delay 4 CV Attn", "%", 0, 100);

                delay = new CBuffer();
        }

        int calc_delay_cv_factor(float cv, float cv_attn, int a=0, int b=0) {
            return (1 + (cv/5) * 2 * cv_attn);
        }

        /*
         * in params[delay_knob].getValue(), expecting 0..1, map to the range of the buffer length
         * in inputs[cv_input].getVoltage(), expecting +/- 5V, map to +/- 1 maybe?
         * in params[cv_knob].getValue(), expecting +/- 1
         */
        int calc_base_samples(int delay_knob, int cv_input, int cv_knob) {
            int samples = params[delay_knob].getValue() * delay->mBufferSize;
            if(inputs[cv_input].isConnected())
                samples = samples * calc_delay_cv_factor(inputs[cv_input].getVoltage(), params[cv_knob].getValue());                

            return clamp(samples, 0, delay->mBufferSize);
        }

        /*
         * in params[delay_knob].getValue(), expecting +/- 2 to allow range from halving to doubling, CHECK THE MATH ON THIS
         * in inputs[cv_input].getVoltage(), expeccting +/- 5V, map to +/- 1 maybe?
         * in params[cv_knob].getValue(), expecting +/- 1
         */
        int calc_sub_samples(int base_samples, int delay_knob, int cv_input, int cv_knob) {
            float delay_ratio = params[delay_knob].getValue();
            int samples = base_samples * (1+delay_ratio);

            if(inputs[cv_input].isConnected())
                samples = samples * calc_delay_cv_factor(inputs[cv_input].getVoltage(), params[cv_knob].getValue());                

            return clamp(samples, 0, delay->mBufferSize);
        }

        int counter = 0;
        void process(const ProcessArgs &args) override {

//            float delay_cv;
//            float delay_cv_attn;

            int delay1_samples = calc_base_samples(DELAY1_KNOB, DELAY1_CV_INPUT, DELAY1_CV_KNOB);
            int delay2_samples = calc_sub_samples(delay1_samples, DELAY2_KNOB, DELAY2_CV_INPUT, DELAY2_CV_KNOB);
            int delay3_samples = calc_sub_samples(delay2_samples, DELAY3_KNOB, DELAY3_CV_INPUT, DELAY3_CV_KNOB);
            int delay4_samples = calc_sub_samples(delay3_samples, DELAY4_KNOB, DELAY4_CV_INPUT, DELAY4_CV_KNOB);

            /*
                read the inputs
             */
            float in1 = inputs[IN1_INPUT].getVoltage(); // audio input, expecting +/-5
            delay->write(in1);


            // if(counter++ > 100000) {
            //     std::cout << "delay: " << delay1_samples << " delay2: " << delay2_samples << "\n";
            //     counter = 0;
            // }

            outputs[DELAY1_OUTPUT].setVoltage(delay->read_num_samples(delay1_samples));
            outputs[DELAY2_OUTPUT].setVoltage(delay->read_num_samples(delay2_samples));
            outputs[DELAY3_OUTPUT].setVoltage(delay->read_num_samples(delay3_samples));
            outputs[DELAY4_OUTPUT].setVoltage(delay->read_num_samples(delay4_samples));
                                               


        }  
};



struct WeirdDelayWidget : ModuleWidget {
        WeirdDelayWidget(WeirdDelay *module) {
                setModule(module);
                // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WeirdDelay.svg")));
                setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/UbDelay2.svg")));

                addChild(createWidget<ScrewBlack>(Vec(15, 0)));
                addChild(createWidget<ScrewBlack>(Vec(15, 365)));

                // addParam(createParam<Davies1900hWhiteKnob>(Vec(19, 32), module, WeirdDelay::DELAY1_KNOB));
                int c1 = 13;
                int c2 = 45;
                int c3 = 75;
                int c4 = 112;

                int row = 40;
                int h = 30;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, WeirdDelay::DELAY1_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, WeirdDelay::DELAY1_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, WeirdDelay::DELAY1_KNOB));
                addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, WeirdDelay::DELAY1_OUTPUT));

                h += row;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, WeirdDelay::DELAY2_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, WeirdDelay::DELAY2_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, WeirdDelay::DELAY2_KNOB));
                addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, WeirdDelay::DELAY2_OUTPUT));

                h += row;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, WeirdDelay::DELAY3_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, WeirdDelay::DELAY3_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, WeirdDelay::DELAY3_KNOB));
                addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, WeirdDelay::DELAY3_OUTPUT));

                h += row;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, WeirdDelay::DELAY4_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, WeirdDelay::DELAY4_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, WeirdDelay::DELAY4_KNOB));
                addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, WeirdDelay::DELAY4_OUTPUT));                                


                // addParam(createParam<Davies1900hWhiteKnob>(Vec(19, 137), module, WeirdDelay::CH3_PARAM));
                // addInput(createInput<PJ301MPort>(Vec(7, 155), module, WeirdDelay::DELAY2_CV_INPUT));
                // addParam(createParam<Davies1900hWhiteKnob>(Vec(19, 190), module, WeirdDelay::CH4_PARAM));

                addInput(createInput<PJ301MPort>(Vec(7, 330), module, WeirdDelay::IN1_INPUT));

                // addOutput(createOutput<PJ301MPort>(Vec(7, 281), module, WeirdDelay::OUT2_OUTPUT));
                // addOutput(createOutput<PJ301MPort>(Vec(43, 281), module, WeirdDelay::OUT3_OUTPUT));

                // addOutput(createOutput<PJ301MPort>(Vec(20, 300), module, WeirdDelay::OUT4_OUTPUT));

                // addOutput(createOutput<PJ301MPort>(Vec(7, 324), module, WeirdDelay::OUT5_OUTPUT));
                // addOutput(createOutput<PJ301MPort>(Vec(43, 324), module, WeirdDelay::OUT6_OUTPUT));

                // addChild(createLight<MediumLight<GreenRedLight>>(Vec(32.7, 310), module, WeirdDelay::OUT_POS_LIGHT));
        }
};


Model *modelWeirdDelay = createModel<WeirdDelay, WeirdDelayWidget>("WeirdDelay");

