#include "plugin.hpp"
#include "CBuffer.hpp"

/*
	Notes: I'm pretty excited about the way this turned out on the first pass. Some ideas:
	- feedback: could have just level 1 feedback or could try to have the different outputs
	  exhibit different feedback based on their position or a knob, have to think about this...

*/


struct SimpleDelay : Module {
        enum ParamIds {
                DELAY_KNOB,
                DELAY_CV_KNOB,
                FB_KNOB,
                FB_CV_KNOB,
                MIX_KNOB,
                MIX_CV_KNOB,
                NUM_PARAMS
        };
        enum InputIds {
                IN_INPUT,
                DELAY_CV_INPUT,
                FB_CV_INPUT,
                MIX_CV_INPUT,
                NUM_INPUTS
        };
        enum OutputIds {
                OUT_OUTPUT,
                NUM_OUTPUTS
        };
        enum LightIds {
                // OUT_POS_LIGHT,
                // OUT_NEG_LIGHT,
                NUM_LIGHTS
        };

        CBuffer* delay;
        int sample_rate = 44100;
        float max_delay = 1000.0;

        SimpleDelay() {
                config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
                // void configParam(int paramId, float minValue, float maxValue, float defaultValue, std::string label = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f)
                configParam(DELAY_KNOB, 0.0, max_delay, 0.0, "Delay", "ms");
                configParam(DELAY_CV_KNOB, -1.0, 1.0, 0.0, "Delay CV Atten", "wha");
                configParam(FB_KNOB, -1.0, 1.0, 0.0, "Feedback", "%");
                configParam(FB_CV_KNOB, -1.0, 1.0, 0.0, "Feedback Atten", "th");
                configParam(MIX_KNOB, 0.0, 1.0, 0.5, "Wet/Dry Mix", "%");
                configParam(MIX_CV_KNOB, -1.0, 1.0, 0.0, "Mix Atten", "hk");

                delay = new CBuffer(96000);
        }

        int counter = 0;
        void process(const ProcessArgs &args) override {

            float delay_ms = params[DELAY_KNOB].getValue();           // 0-max_delay ms
            float delay_cv_attn = params[DELAY_CV_KNOB].getValue();   // +/- 1
            float delay_cv = inputs[DELAY_CV_INPUT].getVoltage() / 5; // +/- 1
            // this is bullshit, not sure what model to follow for it V/Oct-ish
            float delay_final = delay_ms + ((max_delay/2) * delay_cv * delay_cv_attn); 
            int delay_samples = delay_final*(sample_rate/1000.0);


            float fb = params[FB_KNOB].getValue();              // +/- 1
            float fb_cv_attn = params[FB_CV_KNOB].getValue();   // +/- 1
            float fb_cv = inputs[FB_CV_INPUT].getVoltage() / 5; // +/- 1
            float fb_final = fb + fb_cv * fb_cv_attn;
            


            float mix = params[MIX_KNOB].getValue();
            float mix_cv_attn = params[MIX_CV_KNOB].getValue();
            float mix_cv = inputs[MIX_CV_INPUT].getVoltage();
            float mix_final = mix + mix_cv * mix_cv_attn;



            float input = inputs[IN_INPUT].getVoltage(); // audio input, expecting +/-5

            float delay_out = delay->read_num_samples(delay_samples);

            delay->write(input + fb_final * delay_out);


            // if(counter++ > 100000) {
            //     std::cout << "delay: " << delay1_samples << " delay2: " << delay2_samples << "\n";
            //     counter = 0;
            // }


            outputs[OUT_OUTPUT].setVoltage(input*(1-mix_final) + delay_out*mix_final);
                                               


        }  
};



struct SimpleDelayWidget : ModuleWidget {
        SimpleDelayWidget(SimpleDelay *module) {
                setModule(module);
                setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/UbDelay2.svg")));

                addChild(createWidget<Knurlie>(Vec(15, 0)));
                addChild(createWidget<Knurlie>(Vec(15, 365)));

                // addParam(createParam<Davies1900hWhiteKnob>(Vec(19, 32), module, SimpleDelay::DELAY1_KNOB));
                int c1 = 13;
                int c2 = 45;
                int c3 = 75;
                int c4 = 112;

                int row = 40;
                int h = 30;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, SimpleDelay::DELAY_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, SimpleDelay::DELAY_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, SimpleDelay::DELAY_KNOB));
                // addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, SimpleDelay::DELAY1_OUTPUT));

                h += row;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, SimpleDelay::FB_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, SimpleDelay::FB_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, SimpleDelay::FB_KNOB));
                // addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, SimpleDelay::DELAY2_OUTPUT));

                h += row;
                addInput(createInput<PJ301MPort>(Vec(c1 , h), module, SimpleDelay::MIX_CV_INPUT));
                addParam(createParam<Trimpot>(Vec(c2, h), module, SimpleDelay::MIX_CV_KNOB));
                addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, SimpleDelay::MIX_KNOB));
                // addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, SimpleDelay::DELAY3_OUTPUT));

                h += row;
                // addInput(createInput<PJ301MPort>(Vec(c1 , h), module, SimpleDelay::DELAY4_CV_INPUT));
                // addParam(createParam<Trimpot>(Vec(c2, h), module, SimpleDelay::DELAY4_CV_KNOB));
                // addParam(createParam<Rogan1PBlue>(Vec(c3, h), module, SimpleDelay::DELAY4_KNOB));
                addOutput(createOutput<PJ301MPort>(Vec(c4, h), module, SimpleDelay::OUT_OUTPUT));                                


                // addParam(createParam<Davies1900hWhiteKnob>(Vec(19, 137), module, SimpleDelay::CH3_PARAM));
                // addInput(createInput<PJ301MPort>(Vec(7, 155), module, SimpleDelay::DELAY2_CV_INPUT));
                // addParam(createParam<Davies1900hWhiteKnob>(Vec(19, 190), module, SimpleDelay::CH4_PARAM));

                addInput(createInput<PJ301MPort>(Vec(7, 330), module, SimpleDelay::IN_INPUT));

                // addOutput(createOutput<PJ301MPort>(Vec(7, 281), module, SimpleDelay::OUT2_OUTPUT));
                // addOutput(createOutput<PJ301MPort>(Vec(43, 281), module, SimpleDelay::OUT3_OUTPUT));

                // addOutput(createOutput<PJ301MPort>(Vec(20, 300), module, SimpleDelay::OUT4_OUTPUT));

                // addOutput(createOutput<PJ301MPort>(Vec(7, 324), module, SimpleDelay::OUT5_OUTPUT));
                // addOutput(createOutput<PJ301MPort>(Vec(43, 324), module, SimpleDelay::OUT6_OUTPUT));

                // addChild(createLight<MediumLight<GreenRedLight>>(Vec(32.7, 310), module, SimpleDelay::OUT_POS_LIGHT));
        }
};


Model *modelSimpleDelay = createModel<SimpleDelay, SimpleDelayWidget>("SimpleDelay");

