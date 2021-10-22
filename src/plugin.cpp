#include "plugin.hpp"

#define DR_WAV_IMPLEMENTATION
#include "../dep/dr_wav.h"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	p->addModel(modelSimpleDelay);
	p->addModel(modelWeirdDelay);
	p->addModel(modelADSR);
    p->addModel(modelTrackHold);
    p->addModel(modelFirstOrderLab);
    // p->addModel(modelResonator);
    p->addModel(modelFirstOrderBiQuad);
    p->addModel(modelSecondOrderBiQuad);
    p->addModel(modelAnaLogic);
    p->addModel(modelAnaLogic2);
    p->addModel(modelOsc1);
    p->addModel(modelAPFilter);
    p->addModel(modelReverb1);
    p->addModel(modelReverbGSmall);
    p->addModel(modelSpectralMix);
    p->addModel(modelWaves);
    p->addModel(modelStrings);
    p->addModel(modelResBody);
    p->addModel(modelChordo);
    p->addModel(modelPlucktX3);
}
