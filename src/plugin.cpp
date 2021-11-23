#include "plugin.hpp"

#define DR_WAV_IMPLEMENTATION
#include "../dep/dr_wav.h"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

    p->addModel(modelAnaLogic2);
    // p->addModel(modelGrains);
    p->addModel(modelGrainTest);
    p->addModel(modelGrainPulse);
    p->addModel(modelPulseTrain);
    p->addModel(modelPulseTrain2);
    p->addModel(modelInpulse);
    p->addModel(modelLoopy);
    p->addModel(modelPluckt);
    p->addModel(modelPlucktX3);
    p->addModel(modelSpectralMix);
    p->addModel(modelTrackHold);
    p->addModel(modelReverbGSmall);
    p->addModel(modelReverb1);
    p->addModel(modelChordo);
    p->addModel(modelWaves);
    p->addModel(modelOsc1);
	p->addModel(modelADSR);
    p->addModel(modelAnaLogic);
    p->addModel(modelAPFilter);
    p->addModel(modelResBody);
    p->addModel(modelFirstOrderLab);
    p->addModel(modelFirstOrderBiQuad);
    p->addModel(modelSecondOrderBiQuad);
	p->addModel(modelSimpleDelay);
	p->addModel(modelWeirdDelay);
}
