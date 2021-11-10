#ifndef GRAIN_HPP
#define GRAIN_HPP

class Grain {
    int _length = 0;
    int _idx = 0;
    // float _freq=440.f;
    float _phase = 0.f;
    float _phaseInc = 0.f;
    // int _loopDelay = -1;
    // int _loopDelayCountdown = -1;

public:
    Grain(float freq, int length, int sampleRate=44100) :
        _length(length),
        _phaseInc(2.f * M_PI * freq / sampleRate)
    {}

    Grain(const Grain& grain) :
        _length(grain._length),
        _idx(grain._idx),
        _phase(grain._phase),
        _phaseInc(grain._phaseInc)
    {}

    ~Grain() {
        std::cout << "Grain Death. Cycles left: " << _length - _idx << std::endl;
    }

    float nextSample() {
        if(_idx <= _length) {
            // grain is running
            float wav = _nextWaveSample();
            float env = _nextEnvelopeValue();
            return wav * env;
        }
        return 0.f;
    }

    bool running() { return _idx <= _length; }
    void restart() { _idx = 0; }

protected:
    float _nextWaveSample() {
        float out = sin(_phase);

        _phase += _phaseInc;
        if(_phase >= 2.f * M_PI) {
            _phase -= 2.f * M_PI;
        }

        return out;
    }

    float _nextEnvelopeValue() {
        float mid = _length / 2;
        float out = (_idx <= mid) ? _idx / mid : 1 - (_idx-mid) / mid;
        _idx++;
        return out;
    }

};

#endif