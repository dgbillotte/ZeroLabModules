#include "plugin.hpp"


Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	p->addModel(modelSimpleDelay);
	p->addModel(modelWeirdDelay);
	p->addModel(modelADSR);
}
