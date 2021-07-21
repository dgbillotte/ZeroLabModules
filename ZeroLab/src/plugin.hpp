#include "rack.hpp"


using namespace rack;


extern Plugin *pluginInstance;


extern Model *modelSimpleDelay;
extern Model *modelWeirdDelay;
extern Model *modelADSR;


struct Knurlie : SVGScrew {
	Knurlie() {
		sw->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knurlie.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};
