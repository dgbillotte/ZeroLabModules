#ifndef FILTER_HPP
#define FILTER_HPP

class Filter {
protected:
    float _freq;
    int _sample_rate;
    int _dirty = true;
    
public:
    Filter(float freq=440, int sample_rate=44100) :
        _freq(freq), _sample_rate(sample_rate)
    {}

    // freq getter/setter
    float freq() { return _freq; }
    void freq(float f) {
        if(_freq == f)
            return;
        _freq = f;
        _dirty = true;
    }
    
    // sample rate getter/setter
    int sampleRate() { return _sample_rate; }
    void sampleRate(int sr) {
        if(_sample_rate == sr)
            return;
        _sample_rate = sr;
        _dirty = true;
    }
    
    // apply the filter to a single sample
    float process(float input) {
        if(_dirty) {
            computeCoefficients();
            _dirty = false;
        }
        return processInput(input);
    }
    
protected:
    virtual float processInput(float input) { return input; }
    virtual void computeCoefficients() { ; }
};

//--------------------------------------------------------
class SimpleOnePoleLPF : public Filter {
    float a0=0, b1=0, y1=0;
public:
    using Filter::Filter;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float gamma = 2 - cos(theta);
        b1 = sqrt(gamma*gamma -1) - gamma;
        a0 = 1 + b1;
    }
    float processInput(float x0) override {
        float y0 = a0*x0 - b1*y1;
        y1 = y0;
        return y0;
    }
};

//--------------------------------------------------------
class BiQuad : public Filter {
public:
    BiQuad(float freq=440, int sample_rate=44100, float q=0) :
        Filter(freq, sample_rate), _q(q)
    {}    

    // Q getter/setter
    float q() { return _q; }
    void q(float q) {
        if(_q == q)
            return;
        _q = q;
        _dirty = true;
    }

protected:
    float _q=0;
    float x1=0, x2=0, y1=0, y2=0;
    float a0=0, a1=0, a2=0, b1=0, b2=0;
    
    void setDelays(float x0, float y0) {
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
    }
    
    float processInput(float x0) override {
        float y0 = a0*x0 + a1*x1 + a2*x2 - b1*y1 - b2*y2;
        setDelays(x0, y0);
        return y0;
    }
};

//--------------------------------------------------------
class OnePoleLPF : public BiQuad {
public:
    using BiQuad::BiQuad;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float gamma = cos(theta)/(1 + sin(theta));
        a0 = (1 - gamma)/2;
        a1 = a0;
        a2 = 0;
        b1 = -gamma;
        b2 = 0;
    }
};

//--------------------------------------------------------
class OnePoleHPF : public BiQuad {
public:
    using BiQuad::BiQuad;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float gamma = cos(theta)/(1 + sin(theta));
        a0 = (1 + gamma)/2;
        a1 = -a0;
        a2 = 0;
        b1 = -gamma;
        b2 = 0;
    }
};

//--------------------------------------------------------
class TwoPoleLPF : public BiQuad {
public:
    using BiQuad::BiQuad;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float d = 1/_q;
        float beta = 0.5*(1 - d*sin(theta)/2) / (1 + d*sin(theta)/2);
        float gamma = (0.5 + beta)*cos(theta);
        
        a1 = 0.5 + beta - gamma;
        a0 = a1/2;
        a2 = a0;
        b1 = -2*gamma;
        b2 = 2*beta;
    }
};

//--------------------------------------------------------
class TwoPoleHPF : public BiQuad {
public:
    using BiQuad::BiQuad;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float d = 1/_q;
        float beta = 0.5*(1 - d*sin(theta)/2) / (1 + d*sin(theta)/2);
        float gamma = (0.5 + beta)*cos(theta);
        
        a1 = 0.5 + beta + gamma;
        a0 = a1/2;
        a1 = -a1;
        a2 = a0;
        b1 = -2*gamma;
        b2 = 2*beta;
    }
};

//--------------------------------------------------------
class TwoPoleBPF : public BiQuad {
public:
    using BiQuad::BiQuad;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float tan_theta2q = tan(clamp(theta/(2*_q), 0.0f, nextafter(M_PI/2, 0.0f)));
        float beta = 0.5*(1 - tan_theta2q)/(1 + tan_theta2q);
        float gamma = (0.5 + beta)*cos(theta);
        
        a0 = 0.5 - beta;
        a1 = 0;
        a2 = -a0;
        b1 = -2*gamma;
        b2 = 2*beta;
    }
};

//--------------------------------------------------------
class TwoPoleBSF : public BiQuad {
public:
    using BiQuad::BiQuad;

protected:
    void computeCoefficients() override {
        float theta = 2*M_PI*_freq/_sample_rate;
        float tan_theta2q = tan(clamp(theta/(2*_q), 0.0f, nextafter(M_PI/2, 0.0f)));
        float beta = 0.5*(1 - tan_theta2q)/(1 + tan_theta2q);
        float gamma = (0.5 + beta)*cos(theta);
        
        a0 = 0.5 + beta;
        a1 = -2 * gamma;
        a2 = a0;
        b1 = -2*gamma;
        b2 = 2*beta;
    }
};

#endif