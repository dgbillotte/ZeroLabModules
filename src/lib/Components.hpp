
struct HexScrew : SvgScrew {
	HexScrew() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/ScrewHex.svg")));
		box.size = sw->box.size;
	}
};

struct AudioInputJack : SvgPort {
	AudioInputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/JackBlue.svg")));
	}
};

struct AudioOutputJack : SvgPort {
	AudioOutputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/JackOrange.svg")));
	}
};

struct CVInputJack : SvgPort {
	CVInputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/JackGray.svg")));
	}
};

struct CVOutputJack : SvgPort {
	CVOutputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/JackRed.svg")));
	}
};