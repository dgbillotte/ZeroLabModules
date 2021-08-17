#include "rack.hpp"


using namespace rack;


extern Plugin *pluginInstance;


extern Model *modelSimpleDelay;
extern Model *modelWeirdDelay;
extern Model *modelADSR;
extern Model* modelTrackHold;
extern Model* modelFirstOrderLab;
extern Model* modelResonator;
extern Model* modelFirstOrderBiQuad;
extern Model* modelSecondOrderBiQuad;
extern Model* modelAnaLogic;

//struct Knurlie : app::SilverScrew {
//	Knurlie() {
//		sw->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knurlie.svg"));
//		sw->wrap();
//		box.size = sw->box.size;
//	}
//};
