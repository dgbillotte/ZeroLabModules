#ifndef PULSAR_TRAIN_HPP
#define PULSAR_TRAIN_HPP

#include "Pulsar.hpp"

class PulsarTrain;
typedef std::shared_ptr<PulsarTrain> PulsarTrainPtr;

class PulsarTrain {
    Pulsar _pulsar;
    // size_t _pulsarLength;
    size_t _trainLength;
    size_t _trainEnvKnee;
    float _trainEnvVal;
    float _trainEnvUpInc;
    float _trainEnvDownInc;
    size_t _idx = 0;


public:
    PulsarTrain(BasicOscPtr osc,
                LUTEnvelopePtr env,
                size_t pulsarLength,
                float pulsarDuty,
                size_t trainLength,
                float trainEnvKnee) :
        _pulsar(osc, env, pulsarLength, pulsarDuty),
        // _pulsarLength(pulsarLength),
        _trainLength(trainLength),
        _trainEnvKnee(_trainLength * trainEnvKnee),
        _trainEnvVal((_trainEnvKnee == 0) ? 1.f : 0.f),
        _trainEnvUpInc(1.f / (_trainEnvKnee)),
        _trainEnvDownInc(-1.f / (_trainLength - _trainEnvKnee - 1))
    {}

    float nextSample() {
        if(_idx >= _trainLength)
            return 0.f;

        float out = _pulsar.nextSample() * _trainEnvVal;
        // std::cout << "idx: " << _idx << ", env: " << _trainEnvVal << std::endl;
        _trainEnvVal += (_idx++ < _trainEnvKnee) ? _trainEnvUpInc : _trainEnvDownInc;
        return out;
    }

    bool isRunning() {
        return _idx < _trainLength;
    }
};


#endif