#include "rack.hpp"

using namespace rack;


extern Plugin *pluginInstance;


extern Model *modelSimpleDelay;
extern Model *modelWeirdDelay;
extern Model *modelADSR;
extern Model* modelTrackHold;
extern Model* modelFirstOrderLab;
// extern Model* modelResonator;
extern Model* modelFirstOrderBiQuad;
extern Model* modelSecondOrderBiQuad;
extern Model* modelAnaLogic;
extern Model* modelAnaLogic2;
extern Model* modelOsc1;
extern Model* modelAPFilter;
extern Model* modelReverb1;
extern Model* modelReverbGSmall;
extern Model* modelSpectralMix;
extern Model* modelWaves;
extern Model* modelPluckt;
extern Model* modelResBody;
extern Model* modelChordo;
extern Model* modelPlucktX3;
extern Model* modelInpulse;
extern Model* modelLoopy;
extern Model* modelGrains;
extern Model* modelGrainTest;

//struct Knurlie : app::SilverScrew {
//	Knurlie() {
//		sw->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knurlie.svg"));
//		sw->wrap();
//		box.size = sw->box.size;
//	}
//};
